#ifndef WIFI_STA_H
#define WIFI_STA_H
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_wifi_netif.h"
// Event group variable used by multiple file
extern EventGroupHandle_t e_wifi_event_group;

/**
 * @brief Event group bits for WiFi events
 */
#define WIFI_STA_CONNECTED_BIT  BIT0
#define WIFI_STA_DISCONNECT     BIT1
#define WIFI_STA_STOP           BIT2

#define WIFI_STA_SCAN_INIT      BIT10
#define WIFI_STA_SCAN_START     BIT11
#define WIFI_STA_SCAN_DONE      BIT12
/**
 * @brief Event group bits for IP events
 */
#define WIFI_STA_IPV4_OBTAINED_BIT  BIT20
#define WIFI_STA_IPV6_OBTAINED_BIT  BIT21

/**
 * @brief Event group bits for User Selection
 */
#define WIFI_CHOSEN_STA_RECONENCT BIT30


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

 /**
  * @brief Scan Wifi in station mode (STA) mode
  * Scan all avaliable Wifi (Except hidden WiFI) 
  * 
  * @param ap_num The value constain the number of scanned WiFi
  * @param ap_record The value constain scanned WiFis
  * 
  * @return 
  * - ESP_OK : On sucess
  * - Other errors on failure
  */

void wifi_sta_scan_init_default();
void wifi_sta_scan_init(char *SSID, uint8_t, bool show_hidden);

esp_err_t wifi_sta_scan_start();
esp_err_t wifi_sta_scan_get_ap_num(uint16_t *ap_num);
esp_err_t wifi_sta_scan_read(wifi_ap_record_t **ap_record);

bool is_wifi_sta_scan_done();
bool is_wifi_sta_scan_start();

#endif // WIFI_STA_H