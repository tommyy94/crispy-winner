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
#define MAIN_WLAN_AUTH                    M2M_WIFI_SEC_WPA_PSK /**< Security manner */
#define MAIN_WIFI_M2M_PRODUCT_NAME        "NMCTemp"
#define MAIN_WIFI_M2M_SERVER_IP           0xc0a80108 //Provide the TCP Server IP Address in HEX. e.g.: 0xc0=192, 0xa8=168, 0xbc=188, 0x14=20 //0xFFFFFFFF /* 255.255.255.255 */
#define MAIN_WIFI_M2M_SERVER_PORT         (6666)	// Port must be the same between TCP Client and Server
#define MAIN_WIFI_M2M_REPORT_INTERVAL     (1000)

//#define MAIN_WIFI_M2M_BUFFER_SIZE          1460

/** UDP MAX packet count */
#define MAIN_WIFI_M2M_PACKET_COUNT         10

/** Application settings **/
#define APP_WIFI_STATION	1		// 1..5
#define APP_PACKET_COUNT	33		// 33 UDP packets of 1136 bytes each
#define APP_BUFFER_SIZE		1134


extern OS_MAILBOX   socketMboxTbl[MAX_SOCKET];

static SOCKET       sensorListenSocket = -1;
static uint8_t      wifiConnected;


static void wifi_cb(uint8_t u8MsgType, void *pvMsg);
static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg);
static void Wireless_Init(void);
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
        ret = m2m_wifi_handle_events(NULL);
        assert(ret == M2M_SUCCESS);

        OS_TASK_Delay(1000);
    }
}

            // Create socket
            /* ~$ nc -l 6666 */
            if (tcp_client_socket < 0)
            {
                if ((tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                {
                    puts("main : failed to create TCP client socket error!\r\n");


uint8_t sendBuf[3] = { 'X', 'Y', 'Z' };
uint8_t recvBuf[3];
void Sensor_Task(void *arg)
{
    struct sockaddr_in  addr;
    int32_t             ret;
    SOCKET              sensorSocket = -1;
    (void)arg;

    addr.sin_family       = AF_INET;
    addr.sin_port         = _htons(MAIN_WIFI_M2M_SERVER_PORT);
    addr.sin_addr.s_addr  = _htonl(MAIN_WIFI_M2M_SERVER_IP);
    
    /* ~$ nc -l 6666 */
    
    while(1)
    {
        if (wifiConnected == M2M_WIFI_CONNECTED)
        {
            if (sensorListenSocket < 0)
            {
                sensorListenSocket = socket(AF_INET, SOCK_STREAM, 0);
                if (sensorListenSocket >= 0)
                {
                    ret = bind(sensorListenSocket,
                               (struct sockaddr *)&addr,
                               sizeof(struct sockaddr_in));
                    if (ret == SOCK_ERR_NO_ERROR)
                    {
                        /* SOCKET type is signed, but we can receive
                         * only sensorSocket > 0 from the mailbox.
                         */
                        OS_MAILBOX_GetBlocked1(&socketMboxTbl[sensorListenSocket],
                                               (char *)&sensorSocket);
                        assert(sensorSocket <= MAX_SOCKET);
                    }
                    else
                    {
                        close(sensorListenSocket);
                        sensorListenSocket = -1;
                        puts("Sensor_Task: Failed to bind socket!\r\n");
                    }
                }
            }

            if (sensorSocket >= 0)
            {
                ret = send(sensorSocket, sendBuf, sizeof(sendBuf), 0);
                if (ret != SOCK_ERR_NO_ERROR)
                {
                    close(sensorSocket);
                    sensorSocket = -1;
                    puts("Sensor_Task: Failed to send!\r\n");
                }
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
    tstrSocketBindMsg   *pstrBind;
    tstrSocketListenMsg *pstrListen;
    tstrSocketAcceptMsg *pstrAccept;
    int                  ret =-1;

    switch(u8Msg)
    {
        case SOCKET_MSG_BIND:
            pstrBind = (tstrSocketBindMsg *)pvMsg;
            if (pstrBind != NULL)
            {
                if (pstrBind->status == 0)
                {
                    ret = listen(sock, 0);
                    if(ret < 0)
                    {
                        printf("Listen failure! Error = %d\n", ret);
                    }
                }
                else
                {
                    M2M_ERR("bind Failure!\n");
                    close(sock);
                }
            }
            break;
        case SOCKET_MSG_LISTEN:
            pstrListen = (tstrSocketListenMsg *)pvMsg;
            if (pstrListen != NULL)
            {
                if (pstrListen->status == 0)
                {
                    ret = accept(sock, NULL, 0);
                    assert(ret == SOCK_ERR_NO_ERROR);
                }
                else
                {
                    M2M_ERR("listen Failure!\n");
                    close(sock);
                }
            }
            break;
        case SOCKET_MSG_ACCEPT:
            pstrAccept = (tstrSocketAcceptMsg *)pvMsg;
            if (pstrAccept->sock < 0)
            {
                M2M_ERR("accept failure\n");
                close(pstrAccept->sock);
            }
            else
            {
                ret = OS_MAILBOX_Put1(&socketMboxTbl[sock],
                                      (char *)&pstrAccept->sock);
                assert(ret == 0);
            }
            break;
        case SOCKET_MSG_RECV:
            puts("recv ok\r\n");
            break;
        case SOCKET_MSG_SEND:
            puts("send ok\r\n");
        default:
            break;
    }
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
