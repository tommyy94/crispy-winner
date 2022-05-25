/* System includes */
#include <stdio.h>
#include <stdint.h>
#include "wireless.h"
#include "RTOS.h"
#include "SEGGER_RTT.h"

/* ATWINC3400 */
#include "driver/include/m2m_wifi.h"
#include "common/include/nm_common.h"
#include "socket/include/socket.h"
#include "conf_winc.h"
#include "same70.h"

/* User includes */
#include "wireless.h"
#include "logWriter.h"


/** Wi-Fi Settings */
#define MAIN_WLAN_SSID                    "NETGEAR38" //"TP-Link_9446" /**< Destination SSID */
#define MAIN_WLAN_AUTH                    M2M_WIFI_SEC_WPA_PSK /**< Security manner */
#define MAIN_WLAN_PSK                     "cleverbug651" //"84250018" /**< Password for Destination SSID */
#define MAIN_WIFI_M2M_PRODUCT_NAME        "NMCTemp"
#define MAIN_WIFI_M2M_SERVER_IP           0xc0a80107 //Provide the TCP Server IP Address in HEX. e.g.: 0xc0=192, 0xa8=168, 0xbc=188, 0x14=20 //0xFFFFFFFF /* 255.255.255.255 */
#define MAIN_WIFI_M2M_CLIENT_IP           0xc0a8010a //
#define MAIN_WIFI_M2M_SERVER_PORT         (6666)	// Port must be the same between TCP Client and Server
#define MAIN_WIFI_M2M_REPORT_INTERVAL     (1000)

#define MAIN_WIFI_M2M_BUFFER_SIZE          1460

/** UDP MAX packet count */
#define MAIN_WIFI_M2M_PACKET_COUNT         10

/** Message format definitions. */
typedef struct s_msg_wifi_product
{
    uint8_t name[9];
} t_msg_wifi_product;

typedef struct s_msg_wifi_product_main
{
    uint8_t name[9];
} t_msg_wifi_product_main;

/** Message format declarations. */
static t_msg_wifi_product msg_wifi_product =
{
    .name = MAIN_WIFI_M2M_PRODUCT_NAME
};

static t_msg_wifi_product_main msg_wifi_product_main =
{
    .name = MAIN_WIFI_M2M_PRODUCT_NAME
};

static uint8_t      wifiConnected = M2M_WIFI_DISCONNECTED;
static uint8_t      sensorRecv[MAIN_WIFI_M2M_BUFFER_SIZE] = {0};

static SOCKET       controlSocket = -1;
static SOCKET       sensorSocket = -1;

extern OS_MUTEX     wlessMutex;


static void wifi_cb(uint8_t u8MsgType, void *pvMsg);
static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg);
static void Wireless_Init(void);

static void ControlSocketCb(uint8_t u8Msg, void *pvMsg);
static void SensorSocketCb(uint8_t u8Msg, void *pvMsg);

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

        OS_TASK_Delay(1000);
    }
}


void Sensor_Task(void *arg)
{
    struct sockaddr_in  addr;
    int32_t ret;
    (void)arg;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family       = AF_INET;
    addr.sin_port         = _htons(MAIN_WIFI_M2M_SERVER_PORT);
    addr.sin_addr.s_addr  = _htonl(MAIN_WIFI_M2M_CLIENT_IP);
    
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
                    puts("Sensor_Task: failed to create TX UDP client socket error!\r\n");
                    continue;
                }
            }

            OS_MUTEX_LockBlocked(&wlessMutex);

            /* Send client discovery frame. */
            sendto(sensorSocket,
                   &msg_wifi_product,
                   sizeof(t_msg_wifi_product),
                   0,
                   (struct sockaddr *)&addr,
                   sizeof(addr));

            ret = sendto(sensorSocket,
                         &msg_wifi_product_main,
                         sizeof(t_msg_wifi_product_main),
                         0,
                         (struct sockaddr *)&addr,
                         sizeof(addr));
            OS_MUTEX_Unlock(&wlessMutex);

            if (ret == M2M_SUCCESS) 
            {
                puts("Sensor_Task: message sent\r\n");
            }
            else
            {
                puts("Sensor_Task: failed to send status report error!\r\n");
            }
        }
    
        OS_TASK_Delay(1000);
    }
}


void Control_Task(void *arg)
{
    struct sockaddr_in  addr;
    (void)arg;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family       = AF_INET;
    addr.sin_port         = _htons(MAIN_WIFI_M2M_SERVER_PORT);
    addr.sin_addr.s_addr  = _htonl(MAIN_WIFI_M2M_SERVER_IP);
    
    /* ~$ nc -l 6666 */
    
    while(1)
    {
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
    
        OS_TASK_Delay(1000);
    }
}


static void Wireless_Init(void)
{
    tstrWifiInitParam param;
    int8_t            ret;

    puts("Configuring ATWINC3400...\n");

    /* Initialize Wi-Fi parameters structure. */
    memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));
    
    OS_MUTEX_LockBlocked(&wlessMutex);

    /* Initialize Wi-Fi driver with data and status callbacks. */
    param.pfAppWifiCb = wifi_cb;
    ret = m2m_wifi_init(&param);
    if (M2M_SUCCESS != ret)
    {
        printf("main: m2m_wifi_init call error!(%d)\r\n", ret);
        OS_TASK_Terminate(NULL);
    }
  
    /* Init socket module */
    socketInit();
    registerSocketCallback(socket_cb, NULL) ;

    /* Connect to router */
    m2m_wifi_connect((char *)MAIN_WLAN_SSID,
                     sizeof(MAIN_WLAN_SSID),
                     MAIN_WLAN_AUTH,
                     (char *)MAIN_WLAN_PSK,
                     M2M_WIFI_CH_ALL);

    OS_MUTEX_Unlock(&wlessMutex);
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
                puts("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: CONNECTED\r\n");
                m2m_wifi_request_dhcp_client();
            }
            else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED)
            {
                puts("wifi_cb: M2M_WIFI_RESP_CON_STATE_CHANGED: DISCONNECTED\r\n");
                wifiConnected = M2M_WIFI_DISCONNECTED;
                m2m_wifi_connect((char *)MAIN_WLAN_SSID,
                                 sizeof(MAIN_WLAN_SSID),
                                 MAIN_WLAN_AUTH,
                                 (char *)MAIN_WLAN_PSK,
                                 M2M_WIFI_CH_ALL);
            }
            break;
        case M2M_WIFI_REQ_DHCP_CONF:
            pu8IPAddress = (uint8_t *)pvMsg;
            wifiConnected = M2M_WIFI_CONNECTED;
            printf("wifi_cb: M2M_WIFI_REQ_DHCP_CONF: IP is %u.%u.%u.%u\r\n",
                   pu8IPAddress[0],
                   pu8IPAddress[1],
                   pu8IPAddress[2], 
                   pu8IPAddress[3]);
            break;
        default:
            break;
    }
}


/**
 * \brief Callback to get the Data from socket.
 *
 * \param[in] sock socket handler.
 * \param[in] u8Msg socket event type. Possible values are:
 *  - SOCKET_MSG_BIND
 *  - SOCKET_MSG_LISTEN
 *  - SOCKET_MSG_ACCEPT
 *  - SOCKET_MSG_CONNECT
 *  - SOCKET_MSG_RECV
 *  - SOCKET_MSG_SEND
 *  - SOCKET_MSG_SENDTO
 *  - SOCKET_MSG_RECVFROM
 * \param[in] pvMsg is a pointer to message structure. Existing types are:
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
    else
    {
        puts("socket_cb(): Unknown socket!\r\n");
    }
}

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
                puts("ControlSocketCb: bind success!\r\n");
                recvfrom(sock,
                         sensorRecv,
                         MAIN_WIFI_M2M_BUFFER_SIZE,
                         0);
            }
            else
            {
                puts("ControlSocketCb: bind error!\r\n");
            }
            break;
        case SOCKET_MSG_RECVFROM:
            pstrRx = (tstrSocketRecvMsg *)pvMsg;

            printf("Received '%c'\r\n", (char)pstrRx->pu8Buffer[0]);
            if (pstrRx->pu8Buffer && pstrRx->s16BufferSize)
            {
                /* Prepare next buffer reception */
                recvfrom(sock,
                         sensorRecv,
                         MAIN_WIFI_M2M_BUFFER_SIZE,
                         0);
            }
            else
            {
                if (pstrRx->s16BufferSize == SOCK_ERR_TIMEOUT)
                {
                    /* Prepare next buffer reception */
                    recvfrom(sock,
                             sensorRecv,
                             MAIN_WIFI_M2M_BUFFER_SIZE,
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

static void SensorSocketCb(uint8_t u8Msg, void *pvMsg)
{
    (void)u8Msg;
    (void)pvMsg;
}

void ping_cb(uint32 u32IPAddr, uint32 u32RTT, uint8 u8ErrorCode);
void ping_cb(uint32 u32IPAddr, uint32 u32RTT, uint8 u8ErrorCode)
{
    if (!u8ErrorCode)
    {
        puts("ping_cb: PING_SUCCESS\r\n");
        printf("ping_cb: Reply from %x IP: RTT= %dms\r\n", u32IPAddr, u32RTT);
    }
    else if (u8ErrorCode==1)
    {
        puts("ping_cb: PING_ERR_DEST_UNREACH\r\n");
    }
    else if (u8ErrorCode==2)
    {
        puts("ping_cb: PING_ERR_TIMEOUT\r\n");
    }
}


void WiFi_Ping(char *dstIPaddr)
{
    /*Ping request */
    uint32_t dstIP = nmi_inet_addr(dstIPaddr);
    uint8 u8TTL = 250;

    printf("Pinging [%s] with TTL=%d\r\n", dstIPaddr, u8TTL);
    m2m_ping_req(dstIP, u8TTL, ping_cb);
}
