#ifndef WIFI_CONF_DEFAULT_H
#define WIFI_CONF_DEFAULT_H


/** Wi-Fi settings */
#define WLAN2_SSID                "Router"              // /**< Destination SSID */
#define WLAN2_AUTH                M2M_WIFI_SEC_WPA_PSK  // /**< Security manner */
#define WLAN2_PSK                 "password"            // /**< Password for Destination SSID */
#define WIFI_M2M_SERVER_IP        0xC0A80102            // 192.168.1.2
#define WIFI_M2M_CLIENT_IP        0xC0A80103            // 192.168.1.3
#define WIFI_M2M_VIDEO_PORT       (500)
#define WIFI_M2M_SENSOR_PORT      (501)
#define WIFI_M2M_CONTROL_PORT     (502)

#define WIFI_M2M_REPORT_INTERVAL  (1000)
#define WIFI_M2M_BUFFER_SIZE      1400


#endif /* WIFI_CONF_DEFAULT_H */
