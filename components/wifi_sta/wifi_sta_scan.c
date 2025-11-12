#include "wifi_sta.h"
#include "esp_err.h"
#include "esp_private/wifi.h"
#include "freertos/event_groups.h"
#include <inttypes.h>
// Tag for debug messages
static const char* TAG = "WIFI_STA_SCAN";

// Static global variables
static wifi_ap_record_t *s_ap_record = NULL;
static uint16_t s_ap_num;
static char* s_scan_ssid = NULL;
static uint8_t s_channel = 0;
static bool s_show_hidden = false;
/*******************************************************************
 * Public function implement
 */
void wifi_sta_scan_init_default(){
    wifi_sta_scan_init(NULL,0,false);
}

void wifi_sta_scan_init(char *SSID, uint8_t channel, bool show_hidden)
{
    ESP_LOGI (TAG, "Scan Init");
    s_scan_ssid = SSID;
    s_channel = channel;
    s_show_hidden = show_hidden;
    xEventGroupSetBits (e_wifi_event_group,WIFI_STA_SCAN_INIT);
}

esp_err_t wifi_sta_scan_start(){
    EventBits_t uxBit = xEventGroupGetBits (e_wifi_event_group);
    if (!(uxBit & WIFI_STA_SCAN_INIT)){
        ESP_LOGE (TAG, "STA_SCAN is not initialized");
        return ESP_FAIL;
    }
    const wifi_scan_config_t scan_config = {
                        .ssid = (uint8_t*)s_scan_ssid, // Scan all SSID
                        .bssid = NULL, // Scan all BSSID
                        .channel = s_channel,   // Scan all channel
                        .show_hidden = s_show_hidden,   // Not show hidden network
                        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
                        .scan_time.active.max = 0,
                        .scan_time.active.min = 0 //  min=0, max=0: scan dwells on each channel for 120 ms.
                        };
    esp_err_t esp_ret = esp_wifi_scan_start(&scan_config,false); // Non blocking
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to scan wifi");
        return ESP_FAIL;
    }
    xEventGroupSetBits (e_wifi_event_group,WIFI_STA_SCAN_START);
    xEventGroupClearBits (e_wifi_event_group, WIFI_STA_SCAN_DONE);
    return ESP_OK;
}

/**
 * @brief Read scanned wifi
 * ! You must call wifi_sta_scan_start and wait for scanning done function before call this function
 */
esp_err_t wifi_sta_scan_get_ap_num(uint16_t *ap_num){
    EventBits_t uxBit = xEventGroupGetBits(e_wifi_event_group);
    if (!(uxBit & WIFI_STA_SCAN_START)){
        ESP_LOGE (TAG, "Scanning wifi is not start");
        return ESP_FAIL;        
    }
    if (!(uxBit & WIFI_STA_SCAN_DONE)){
        ESP_LOGE (TAG, "Scanning wifi is not done");
        return ESP_FAIL;
    }
    esp_err_t esp_ret = esp_wifi_scan_get_ap_num(&s_ap_num);
    (*ap_num) = s_ap_num;
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to get the number of scanned AP");
        return ESP_FAIL;
    }
    ESP_LOGI (TAG, "Scanned sucessfully with %d scanned AP",s_ap_num);
    return ESP_OK;
}

/**
 * @brief Read scanned wifi
 * ! You must read num ap and allocate memory yourself depend on the number_of_ap
 */
esp_err_t wifi_sta_scan_read(wifi_ap_record_t **ap_record){
    // Allocate array to hold return ap_record 
    if (ap_record == NULL){
        ESP_LOGE (TAG, "Memory is not allocated for AP_RECORD");
        return ESP_FAIL;
    }
    s_ap_record = (wifi_ap_record_t*) malloc(sizeof(wifi_ap_record_t) * s_ap_num);
    esp_err_t esp_ret = esp_wifi_scan_get_ap_records (&s_ap_num,s_ap_record); 
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to get AP record");
        if (s_ap_record != NULL){
            // Delete previous the scan
            ESP_LOGI (TAG, "Free previously allocated");
            free ((void*)s_ap_record);    
        }
        return ESP_FAIL;
    }
    for (int i = 0; i < s_ap_num ; i++ ){
        (*ap_record)[i] = s_ap_record[i];
        ESP_LOGI (TAG, "Scanned wifi: %d", i);
        ESP_LOGI (TAG, "SSID: %s", s_ap_record[i].ssid);
        ESP_LOGI (TAG, "Auth Mode %d", s_ap_record[i].authmode);
    }
    if (s_ap_record != NULL){
        // Free s_ap_record
        ESP_LOGI (TAG, "Free previously allocated");
        free ((void*)s_ap_record);    
    }
    xEventGroupClearBits (e_wifi_event_group,WIFI_STA_SCAN_START);
    return ESP_OK;    
}

bool is_wifi_sta_scan_done(){
    EventBits_t uxBit = xEventGroupGetBits(e_wifi_event_group);
    return (uxBit & WIFI_STA_SCAN_DONE);
}

bool is_wifi_sta_scan_start(){
    EventBits_t uxBit = xEventGroupGetBits(e_wifi_event_group);
    return (uxBit & WIFI_STA_SCAN_START);    
}