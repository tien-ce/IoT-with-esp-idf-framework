#ifndef WIFI_STA_H
#define WIFI_STA_H
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_wifi_netif.h"

/**
 * @brief Event group bits for WiFi events
 */
#define WIFI_STA_CONNECTED_BIT      BIT0
#define WIFI_STA_IPV4_OBTAINED_BIT  BIT1
#define WIFI_STA_IPV6_OBTAINED_BIT  BIT2

/**
 * @brief Event group bits for User Selection
 */
#define WIFI_CHOSEN_STA_MODE BIT0
#define WIFI_CHOSEN_AP_MODE BIT1
#define WIFI_CHOSEN_STA_RECONENCT BIT2
#define WIFI_CHOSEN_STA_DISCONNECT BIT3
#define WIFI_CHOSEN_STA_STOP BIT4

/**
 * @brief Initialize Wifi in station (STA) mode
 * Set up the WiFi interface and connection and IP address assignment
 * !! You must call esp_netif_init() and esp_event_loop_create_default() before call this function.
 * 
 * @param[in] event_group Event group handle for WiFi and IP events. 
 * 
 * @return 
 * - ESP_OK : On success
 * - Other errors on failure. See esp_err.h to more information
 */

esp_err_t wifi_sta_init (EventGroupHandle_t event_group);

/**
 * @brief Disable Wifi
 * 
 * @return 
 * - ESP_OK : On sucess
 * - Other errors on failure.
 */

 esp_err_t wifi_sta_stop(void);

#endif // WIFI_STA_H