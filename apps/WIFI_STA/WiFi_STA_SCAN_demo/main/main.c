#include <string.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "wifi_sta.h"

// Settings
static const uint32_t sleep_time_ms = 5000;


// App entrypoint

void app_main (void)
{
    esp_err_t esp_ret;
    EventGroupHandle_t network_event_group;
    // Initialize event group
    network_event_group = xEventGroupCreate();

    // Initialize NVS: ESP32 WiFi driver uses NVS to store WiFi settings
    esp_ret = nvs_flash_init();
    if (esp_ret == ESP_ERR_NVS_NO_FREE_PAGES || esp_ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK (nvs_flash_erase());
        esp_ret = nvs_flash_init();
    }
    if (esp_ret != ESP_OK){
        perror ("ERROR (%d): Failed to initialize NVS");
        abort();
    }

    // (s1.3) Create default event loop that runs in the background
    // Must be running prior to initializing the network driver!
    esp_ret = esp_event_loop_create_default();
    if (esp_ret != ESP_OK) {
        perror ("Error (%d): Failed to create default event loop");
        abort();
    }

    // Initialize network connection (wifi_sta.h)
    esp_ret = wifi_sta_init(network_event_group);
    if (esp_ret != ESP_OK) {
        perror ("Error (%d): Failed to initialize WiFi");
        abort();
    }
    wifi_sta_scan_init_default();
    wifi_sta_scan_start(); // Start scan
    // Super loop
    while (1)
    {
        if (is_wifi_sta_scan_start() && is_wifi_sta_scan_done()){
            uint16_t ap_num;
            esp_err_t esp_ret = wifi_sta_scan_get_ap_num(&ap_num);
            if (esp_ret != ESP_OK){
                perror ("Get the number of scanned AP failed");
                abort();
            }
            wifi_ap_record_t* ap_record = (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t)* ap_num);
            esp_ret = wifi_sta_scan_read(&ap_record);        
            if (esp_ret != ESP_OK){
                perror ("Get scanned APs failed");
                abort();
            }
            printf ("AP\t SSID\t Auth Mode\t\n");
            for (int i=0 ; i<ap_num; i++){
                printf ("%d\t",  i);
                printf ("%s\t", ap_record[i].ssid);
                printf ("%d\t", ap_record[i].authmode);
                printf("\n");
            }
            wifi_sta_scan_start();
            vTaskDelay(sleep_time_ms / portTICK_PERIOD_MS);            
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
                                             
}