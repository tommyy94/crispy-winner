/* System includes */
#include <stdint.h>
#include "wireless.h"
#include "RTOS.h"

/* ATWINC3400 */
#include "driver/include/m2m_wifi.h"
#include "common/include/nm_common.h"
#include "socket/include/socket.h"
#include "conf_winc.h"
#include "same70.h"

/* User includes */
#include "wifi_conf.h"
#include "wireless.h"
#include "err.h"
#include "trace.h"
#include "radiopacket.h"


#define WLESS_EVT_SENSOR                  (1u << 0)
#define WLESS_EVT_VIDEO                   (1u << 1)
#define WLESS_EVT_INVALID                 (0xFFFFFFFFU)


/** Message format definitions. */
//char cp[1400] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec at odio arcu. Morbi eu rhoncus nisl. Duis mattis metus nunc, ac blandit lectus lacinia et. Duis nec mollis justo. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nunc cursus mi eu orci vestibulum, sit amet tristique felis ullamcorper. Praesent porta ultricies est sit amet pretium. In hac habitasse platea dictumst. Duis nisl erat, pharetra a nunc id, tincidunt sagittis tellus. Vivamus velit.";
uint8_t g_recv[WIFI_M2M_BUFFER_SIZE];


static uint8_t      wifiConnected = M2M_WIFI_DISCONNECTED;
static uint8_t      ctrlRecv[WIFI_M2M_BUFFER_SIZE] = {0};
static uint32_t     ipAddr;

static SOCKET       controlSocket = -1;
static SOCKET       sensorSocket  = -1;
static SOCKET       videoSocket   = -1;

extern OS_MUTEX     wlessMutex;
extern OS_EVENT     wlessEvt;

extern char         jpgData[4954];
extern char         jpgDataInv[4954];


static void         wifi_cb(uint8_t u8MsgType, void *pvMsg);
static void         socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg);
static void         ControlSocketCb(uint8_t u8Msg, void *pvMsg);
static void         SensorSocketCb(uint8_t u8Msg, void *pvMsg);
static void         VideoSocketCb(uint8_t u8Msg, void *pvMsg);
static void         ping_cb(uint32 u32IPAddr, uint32 u32RTT, uint8 u8ErrorCode);

static OS_TASKEVENT Wireless_Sock2Evt(SOCKET sock);
static void         Wireless_Init(void);

void WiFi_Ping(char *dstIPaddr);


/**
 * @brief   Task responsible for communication with
 *          the remote controller.
 *
 * @param   pvArg   Unused.
 *
 * @return  None.
 */
void Wireless_Task(void *arg)
{
    int8_t ret;
    (void)arg;

    Wireless_Init();

    while (1)
    {
        OS_MUTEX_LockBlocked(&wlessMutex);
        ret = m2m_wifi_handle_events(NULL);
        assert(ret == M2M_SUCCESS);
        OS_MUTEX_Unlock(&wlessMutex);

        OS_TASK_Delay(10);
    }
}


/**
 * @brief   Map socket to an event
 *
 * @param   sock    Network socket.
 *
 * @retval  return  Event bit.
 */
static OS_TASKEVENT Wireless_Sock2Evt(SOCKET sock)
{

    if (sock == sensorSocket)
    {
        return WLESS_EVT_SENSOR;
    }
    else if (sock == videoSocket)
    {
        return WLESS_EVT_VIDEO;
    }
    else
    {
        return WLESS_EVT_INVALID;
    }
}


/**
 * @brief   Transmit single buffer where maximum size is MTU.
 *
 * @param   sock    UDP socket.
 *
 * @param   addr    Address struct.
 *
 * @param   buf     Buffer to transmit.
 *
 * @param   len     Transmission length.
 *
 * @retval  ret     Transmission succeeded.
 */
    uint8_t             empty[WIFI_M2M_BUFFER_SIZE];
static bool Wireless_TransmitUnit(
    SOCKET              sock,
    struct sockaddr_in *addr,
    char                buf[],
    uint32_t            len)
{
    int32_t             ret;
    OS_TASKEVENT        evtMask;
    OS_TASKEVENT        evtBit;

    assert(len <= WIFI_M2M_BUFFER_SIZE);

    OS_MUTEX_LockBlocked(&wlessMutex);
    ret = sendto(sock,
                 buf,
                 len,
                 0,
                 (struct sockaddr *)addr,
                 sizeof(addr));
    recvfrom(sock, empty, sizeof(empty), 0);
    OS_MUTEX_Unlock(&wlessMutex);
    
    if (ret == M2M_SUCCESS)
    {
        evtBit = Wireless_Sock2Evt(sock);
        evtMask = OS_EVENT_GetMaskBlocked(&wlessEvt, evtBit);
        assert((evtMask & evtBit));
    }

    return ret == M2M_SUCCESS;
}


/**
 * @brief   Transmit UDP message.
 *
 * @param   sock    UDP socket.
 *
 * @param   addr    Address struct.
 *
 * @param   buf     Buffer to transmit.
 *
 * @param   len     Transmission length.
 *
 * @retval  ret     Transmission succeeded.
 */
static bool Wireless_Transmit(
    SOCKET              sock,
    struct sockaddr_in *addr,
    char                buf[],
    uint32_t            len)
{
    RadioPacket_t       packet;
    bool                ret;
    uint8_t             seqId = 0;
    uint32_t            mul   = 0;

    /* Serialize the struct we build */
    char               *packetHdrPtr = (char *)&packet;

    assert(sock >= 0);
    assert(addr != NULL);

    do
    {
        BuildRadioPacket(&packet,
                         &buf[RADIOPACKET_PAYLOAD_SIZE * mul],
                         RADIOPACKET_PAYLOAD_SIZE,
                         seqId);

        if (len >= WIFI_M2M_BUFFER_SIZE)
        {
            ret = Wireless_TransmitUnit(sock,
                                        addr,
                                        packetHdrPtr,
                                        WIFI_M2M_BUFFER_SIZE);
            len -= RADIOPACKET_PAYLOAD_SIZE;
        }
        else
        {
            ret = Wireless_TransmitUnit(sock,
                                        addr,
                                        packetHdrPtr,
                                        len + RADIOPACKET_HEADER_SIZE);
            len = 0;
        }

        mul++;

        seqId++;
        /* 1392B x 255 = 354960B (~346KB) should be enough,
         * but add assert to test if it overflows.
         */
        assert(seqId < UINT8_MAX);

        if (ret == false)
        {
            break;
        }
    } while (len > 0);

    return ret;
}


/**
 * @brief   Transmit video stream via UDP.
 *
 * @param   arg     Pointer to arguments.
 *
 * @retval  None.
 */
void Video_Task(void *arg)
{
    struct sockaddr_in  addr;
    bool                ret;

    (void)arg;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family       = AF_INET;
    addr.sin_port         = _htons(WIFI_M2M_VIDEO_PORT);
    addr.sin_addr.s_addr  = _htonl(WIFI_M2M_SERVER_IP);
    
    while(1)
    {
        if (wifiConnected == M2M_WIFI_CONNECTED)
        {
            /* Create socket for Tx UDP */
            if (videoSocket < 0)
            {
                OS_MUTEX_LockBlocked(&wlessMutex);
                videoSocket = socket(AF_INET, SOCK_DGRAM, 0);
                OS_MUTEX_Unlock(&wlessMutex);
                if (videoSocket < 0)
                {
                    TRACE_INFO("Video_Task > failed to create TX UDP client socket error!");
                    continue;
                }
            }

            if (videoSocket >= 0)
            {
                ret = Wireless_Transmit(videoSocket, &addr, jpgData, 5447);
                if (ret == true)
                {
                    TRACE_INFO("Video_Task > message sent");
                }
                else
                {
                    TRACE_INFO("Video_Task > failed to send status report error!");
                }
            }
        }
    
        OS_TASK_Delay(100);
    }
}


/**
 * @brief   Transmit sensor data via UDP.
 *
 * @param   arg     Pointer to arguments.
 *
 * @retval  None.
 */
void Sensor_Task(void *arg)
{
    struct sockaddr_in  addr;
    bool                ret;
    char                test[9] = "sensor\r\n\0";
    (void)arg;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family       = AF_INET;
    addr.sin_port         = _htons(WIFI_M2M_SENSOR_PORT);
    addr.sin_addr.s_addr  = _htonl(WIFI_M2M_SERVER_IP);
    

    while(1)
    {
        if (wifiConnected == M2M_WIFI_CONNECTED)
        {
            /* Create socket for Tx UDP */
            if (sensorSocket < 0)
            {
                OS_MUTEX_LockBlocked(&wlessMutex);
                sensorSocket = socket(AF_INET, SOCK_DGRAM, 0);
                OS_MUTEX_Unlock(&wlessMutex);
                if (sensorSocket < 0)
                {
                    TRACE_INFO("Sensor_Task > failed to create TX UDP client socket error!");
                    continue;
                }
            }

            if (sensorSocket >= 0)
            {
                ret = Wireless_Transmit(sensorSocket, &addr, test, 9);
                if (ret == true)
                {
                    TRACE_INFO("Sensor_Task > message sent");
                }
                else
                {
                    TRACE_INFO("Sensor_Task > failed to send status report error!");
                }
            }
        }
    
        OS_TASK_Delay(1000);
    }
}


/**
 * @brief   Receive control data via TCP.
 *
 * @param   arg     Pointer to arguments.
 *
 * @retval  None.
 */
void Control_Task(void *arg)
{
    struct sockaddr_in  addr;
    (void)arg;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family       = AF_INET;
    addr.sin_port         = _htons(WIFI_M2M_CONTROL_PORT);
    addr.sin_addr.s_addr  = _htonl(WIFI_M2M_SERVER_IP);
    
    /* ~$ nc -l 6666 */
    
    while(1)
    {
        /*
        if (wifiConnected == M2M_WIFI_CONNECTED)
        {
            if (controlSocket < 0)
            {
                OS_MUTEX_LockBlocked(&wlessMutex);
                controlSocket = socket(AF_INET, SOCK_DGRAM, 0);
                if (controlSocket >= 0)
                {
                    bind(controlSocket,
                         (struct sockaddr *)&addr,
                         sizeof(struct sockaddr_in));
                }
                OS_MUTEX_Unlock(&wlessMutex);
            }
        }
        */
    
        OS_TASK_Delay(1000);
    }
}


/**
 * @brief   Initialize wireless module WINC3400.
 *
 * @param   None.
 *
 * @retval  None.
 */
static void Wireless_Init(void)
{
    tstrWifiInitParam param;
    int8_t            ret;

    TRACE_INFO("WINC > Configuring ATWINC3400...");
    
    nm_bsp_init();

    /* Initialize Wi-Fi parameters structure. */
    memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));
    
    OS_MUTEX_LockBlocked(&wlessMutex);

    /* Initialize Wi-Fi driver with data and status callbacks. */
    param.pfAppWifiCb = wifi_cb;
    ret = m2m_wifi_init(&param);
    if (M2M_SUCCESS != ret)
    {
        TRACE_INFO("WINC > m2m_wifi_init call error!");
        OS_TASK_Terminate(NULL);
    }
  
    /* Init socket module */
    socketInit();
    registerSocketCallback(socket_cb, NULL) ;

    /* Connect to router */
    m2m_wifi_connect((char *)WLAN2_SSID,
                     sizeof(WLAN2_SSID),
                     WLAN2_AUTH,
                     (char *)WLAN2_PSK,
                     M2M_WIFI_CH_ALL);

    OS_MUTEX_Unlock(&wlessMutex);

    TRACE_INFO("WINC > ATWINC3400 configured!");
}


/**
 * \brief Callback to get the Wi-Fi status update.
 *
 * \param[in] u8MsgType type of Wi-Fi notification. Possible types are:
 *  - [M2M_WIFI_RESP_CON_STATE_CHANGED](@ref M2M_WIFI_RESP_CON_STATE_CHANGED)
 *  - [M2M_WIFI_RESP_SCAN_DONE](@ref M2M_WIFI_RESP_SCAN_DONE)
 *  - [M2M_WIFI_RESP_SCAN_RESULT](@ref M2M_WIFI_RESP_SCAN_RESULT)
 *  - [M2M_WIFI_REQ_DHCP_CONF](@ref M2M_WIFI_REQ_DHCP_CONF)
 * \param[in] pvMsg A pointer to a buffer containing the notification parameters
 * (if any). It should be casted to the correct data type corresponding to the
 * notification type.
 */
static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
    tstrM2mWifiStateChanged *pstrWifiState;
    uint8_t *pu8IPAddress;

    switch (u8MsgType)
    {
        case M2M_WIFI_RESP_CON_STATE_CHANGED:
            pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
            if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED)
            {
                TRACE_INFO("wifi_cb > M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED");
                m2m_wifi_request_dhcp_client();
            }
            else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED)
            {
                TRACE_INFO("wifi_cb > M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED");
                wifiConnected = M2M_WIFI_DISCONNECTED;
                m2m_wifi_connect((char *)WLAN2_SSID,
                                 sizeof(WLAN2_SSID),
                                 WLAN2_AUTH,
                                 (char *)WLAN2_PSK,
                                 M2M_WIFI_CH_ALL);
            }
            break;
        case M2M_WIFI_REQ_DHCP_CONF:
            pu8IPAddress = (uint8_t *)pvMsg;
            wifiConnected = M2M_WIFI_CONNECTED;
            TRACE_INFO("wifi_cb > M2M_WIFI_REQ_DHCP_CONF: IP is %u.%u.%u.%u\r\n",
                   pu8IPAddress[0],
                   pu8IPAddress[1],
                   pu8IPAddress[2],
                   pu8IPAddress[3]);

                   ipAddr =
                   ((pu8IPAddress[0] & 0xFF) << 0)  |
                   ((pu8IPAddress[1] & 0xFF) << 8)  |
                   ((pu8IPAddress[2] & 0xFF) << 16) |
                   ((pu8IPAddress[3] & 0xFF) << 24);
            break;
        case M2M_WIFI_RESP_SCAN_RESULT:
            TRACE_INFO("wifi_cb > M2M_WIFI_RESP_SCAN_RESULT");
            break;
        case M2M_WIFI_RESP_GET_SYS_TIME:
            TRACE_INFO("wifi_cb > M2M_WIFI_RESP_GET_SYS_TIME");
            break;
        case M2M_WIFI_REQ_SET_BATTERY_VOLTAGE:
            TRACE_INFO("wifi_cb > M2M_WIFI_REQ_SET_BATTERY_VOLTAGE");
            break;
        case M2M_WIFI_RESP_BLE_API_RECV:
            /* Seems like an expected message */
            TRACE_INFO("wifi_cb > M2M_WIFI_RESP_BLE_API_RECV");
            break;
        default:
            TRACE_INFO("wibi_cb > Unknown callback!\r\n");
            break;
    }
}


/**
 * @brief Callback to get the Data from socket.
 *
 * @param[in] sock socket handler.
 * @param[in] u8Msg socket event type. Possible values are:
 *  - SOCKET_MSG_BIND
 *  - SOCKET_MSG_LISTEN
 *  - SOCKET_MSG_ACCEPT
 *  - SOCKET_MSG_CONNECT
 *  - SOCKET_MSG_RECV
 *  - SOCKET_MSG_SEND
 *  - SOCKET_MSG_SENDTO
 *  - SOCKET_MSG_RECVFROM
 * @param[in] pvMsg is a pointer to message structure. Existing types are:
 *  - tstrSocketBindMsg
 *  - tstrSocketListenMsg
 *  - tstrSocketAcceptMsg
 *  - tstrSocketConnectMsg
 *  - tstrSocketRecvMsg
 */
static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
{
    if (sock == controlSocket)
    {
        ControlSocketCb(u8Msg, pvMsg);
    }
    else if (sock == sensorSocket)
    {
       SensorSocketCb(u8Msg, pvMsg);
    }
    else if (sock == videoSocket)
    {
       VideoSocketCb(u8Msg, pvMsg);
    }
    else
    {
        TRACE_INFO("socket_cb > Unknown socket!");
    }
}


/**
 * @brief   Control socket callback. Bind IP address if
 *          needed and start new reception.
 *
 * @param   u8Msg         Unused.
 *
 * @param   pvMsg         Unused.
 *
 * @retval  None.
 */
static void ControlSocketCb(uint8_t u8Msg, void *pvMsg)
{
    tstrSocketBindMsg *pstrBind;
    tstrSocketRecvMsg *pstrRx;
    SOCKET sock = controlSocket;

    OS_MUTEX_LockBlocked(&wlessMutex);

    switch(u8Msg)
    {
        case SOCKET_MSG_BIND:
            pstrBind = (tstrSocketBindMsg *)pvMsg;
            if (pstrBind && pstrBind->status == 0)
            {
                /* Prepare next buffer reception. */
                TRACE_INFO("ControlSocketCb > bind success!");
                recvfrom(sock,
                         ctrlRecv,
                         WIFI_M2M_BUFFER_SIZE,
                         0);
            }
            else
            {
                TRACE_INFO("ControlSocketCb > bind error!");
            }
            break;
        case SOCKET_MSG_RECVFROM:
            pstrRx = (tstrSocketRecvMsg *)pvMsg;

            TRACE_INFO("ControlSocketCb > Received '%c'\r\n", (char)pstrRx->pu8Buffer[0]);
            if (pstrRx->pu8Buffer && pstrRx->s16BufferSize)
            {
                /* Prepare next buffer reception */
                recvfrom(sock,
                         ctrlRecv,
                         WIFI_M2M_BUFFER_SIZE,
                         0);
            }
            else
            {
                if (pstrRx->s16BufferSize == SOCK_ERR_TIMEOUT)
                {
                    /* Prepare next buffer reception */
                    recvfrom(sock,
                             ctrlRecv,
                             WIFI_M2M_BUFFER_SIZE,
                             0);
                }
            }
            break;
        default:
            assert(false);
            break;
    }

    OS_MUTEX_Unlock(&wlessMutex);
}


/**
 * @brief   Sensor socket callback. Signal task socket ok.
 *
 * @param   u8Msg         Unused.
 *
 * @param   pvMsg         Unused.
 *
 * @retval  None.
 */
static void SensorSocketCb(uint8_t u8Msg, void *pvMsg)
{
    (void)u8Msg;
    (void)pvMsg;
    OS_EVENT_SetMask(&wlessEvt, WLESS_EVT_SENSOR);
}


/**
 * @brief   Video socket callback. Signal task socket ok.
 *
 * @param   u8Msg         Unused.
 *
 * @param   pvMsg         Unused.
 *
 * @retval  None.
 */
static void VideoSocketCb(uint8_t u8Msg, void *pvMsg)
{
    (void)u8Msg;
    (void)pvMsg;
    OS_EVENT_SetMask(&wlessEvt, WLESS_EVT_VIDEO);
}
