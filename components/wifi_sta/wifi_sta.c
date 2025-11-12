#include "wifi_sta.h"
#include "esp_err.h"
#include "esp_private/wifi.h"
#include "freertos/event_groups.h"
#include <inttypes.h>
// Extern event group from header file
EventGroupHandle_t e_wifi_event_group = NULL;

// Tag for debug messages
static const char* TAG = "WIFI_STA";

// Static global variables
static esp_netif_t *s_wifi_netif = NULL;
static wifi_netif_driver_t *s_wifi_netif_driver = NULL;
static uint8_t reconnect_count = 0;

/******************************
 * Private functions prototypes
 */

// Private callback functions
static void on_wifi_event   (void* arg,
                            esp_event_base_t event_base,
                            int32_t event_id,
                            void* event_data);

static void on_ip_event     (void* arg,
                            esp_event_base_t event_base,
                            int32_t event_id,
                            void* event_data);

static void wifi_start_cb      (void* esp_netif,
                            esp_event_base_t event_base,
                            int32_t event_id,
                            void* event_data);

static void wifi_stop_cb       (void* esp_netif,
                            esp_event_base_t event_base,
                            int32_t event_id,
                            void* event_data);

static void wifi_connected_cb   (void* esp_netif,
                            esp_event_base_t event_base,
                            int32_t event_id,
                            void* event_data);                            

static void wifi_scan_done_cb ();

static void wifi_disconnected_cb ();

// Private support function                            
static void reconnect_wifi ();


/*******************************
 *  Private functions implementation
 */

// Private callback functions
static void on_wifi_event   (void* arg,
                            esp_event_base_t event_base,
                            int32_t event_id,
                            void* event_data)
{
    switch (event_id){
        case WIFI_EVENT_STA_START:  // Wifi start
            if (s_wifi_netif != NULL){
                wifi_start_cb (s_wifi_netif, event_base, event_id, event_data);                
            }
            break;
        case WIFI_EVENT_STA_STOP:  // Wifi stop
            if (s_wifi_netif != NULL){
                wifi_stop_cb (s_wifi_netif, event_base, event_id, event_data);
            }
            break;
        case WIFI_EVENT_STA_CONNECTED: // Connect to wifi
            // Check valid interface handle
            if (s_wifi_netif == NULL){
                ESP_LOGE (TAG, "Wifi not started: Interface handle is NULL");
                return;
            }
            wifi_connected_cb (s_wifi_netif, event_base, event_id, event_data);
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            wifi_disconnected_cb();
            break;
        case WIFI_EVENT_SCAN_DONE:
            wifi_scan_done_cb();
            break;
        default:
            ESP_LOGI(TAG, "Unexpected behavior %" PRId32 " in WiFi event", event_id);
            break;
    }
}

void on_ip_event    (void *arg,
                     esp_event_base_t event_base,
                     int32_t event_id,
                     void* event_data)
{
    switch (event_id){
        case IP_EVENT_STA_GOT_IP: // DHCP clients successfully get IP address
            ip_event_got_ip_t* event_got_ip = (ip_event_got_ip_t*) event_data;
            esp_netif_ip_info_t *ip_info = &event_got_ip->ip_info;
            ESP_LOGI (TAG, "Wifi IP adress obtained");
            ESP_LOGI (TAG, "IP address: " IPSTR, IP2STR(&ip_info->ip));
            ESP_LOGI (TAG, "Net mask: " IPSTR, IP2STR(&ip_info->netmask));
            ESP_LOGI (TAG, "Gateway IP: " IPSTR, IP2STR(&ip_info->gw));
            xEventGroupSetBits (e_wifi_event_group, WIFI_STA_IPV4_OBTAINED_BIT);
            break;
        case IP_EVENT_STA_LOST_IP:
            ESP_LOGI (TAG, "Wifi lost IP address");
            break;
        default:
            ESP_LOGI(TAG, "Unexpected behavior %" PRId32 " in IP event", event_id);
            break;
    }
}

/***********************************************
 * @brief Set up the wifi interface and start DHCP process
 * Called from on_wifi_event
 */
static void wifi_start_cb (void* esp_netif,
                        esp_event_base_t event_base,
                        int32_t event_id,
                        void* event_data)
{
    uint8_t mac_addr[6] = {0};
    esp_err_t esp_ret;

    // Get esp-netif driver handle
    wifi_netif_driver_t driver = (wifi_netif_driver_t)esp_netif_get_io_driver((esp_netif_t*)esp_netif);
    if (driver == NULL){
        ESP_LOGE (TAG, "Failed to get wifi driver handle");
        return;
    }

    // Get MAC address of WiFi interface
    if ((esp_ret = esp_wifi_get_if_mac(driver,mac_addr))!= ESP_OK){
        ESP_LOGE (TAG, "Failed to get MAC address");
        return;
    }
    ESP_LOGI (TAG, "WiFi MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
              mac_addr[0],
              mac_addr[1],
              mac_addr[2],
              mac_addr[3],
              mac_addr[4],
              mac_addr[5]);
    
    // Register netstack buffer reference and free callback
    esp_ret = esp_wifi_internal_reg_netstack_buf_cb (esp_netif_netstack_buf_ref,esp_netif_netstack_buf_free);
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "ERROR (%d): Netstack callback registration failed", esp_ret);
        return;
    }

    // Set MAC address of wifi interface
    esp_ret = esp_netif_set_mac ((esp_netif_t*)esp_netif, mac_addr);
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to set MAC address fo wifi interface");
        return;
    }

    // Start the wifi interface
    esp_netif_action_start (esp_netif,event_base,event_id,event_data);

    // Connect to wifi
    ESP_LOGI (TAG, "Connect to wifi %s",CONFIG_WIFI_STA_SSID);
    // esp_ret = esp_wifi_connect(); 
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to connect to WiFi");
    }
}

/**
 * @brief 
*/
static void wifi_stop_cb       (void* esp_netif,
                                esp_event_base_t event_base,
                                int32_t event_id,
                                void* event_data)
{
    if (s_wifi_netif != NULL) {
        esp_netif_action_stop(s_wifi_netif, 
                                event_base, 
                                event_id, 
                                event_data);
    }    
}

/**
 * @brief
 */
static void wifi_connected_cb   (void* esp_netif,
                                esp_event_base_t event_base,
                                int32_t event_id,
                                void* event_data)
{
    wifi_event_sta_connected_t *event_sta_connected = (wifi_event_sta_connected_t*) event_data;
    ESP_LOGI (TAG, "Connected to AP");
    ESP_LOGI (TAG, "SSID: %s", (char*) event_sta_connected->ssid);
    ESP_LOGI (TAG, "Channel: %d", event_sta_connected->channel);
    ESP_LOGI (TAG, "Auth mode: %d", event_sta_connected->authmode);
    ESP_LOGI (TAG, "AID: %d", event_sta_connected->aid);

    // Register interface receive callback
    wifi_netif_driver_t driver = esp_netif_get_io_driver(s_wifi_netif);
    if (!esp_wifi_is_if_ready_when_started(driver)) {
        esp_err_t esp_ret = esp_wifi_register_if_rxcb(driver,
                                            esp_netif_receive,
                                            s_wifi_netif);
        if (esp_ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register WiFi RX callback");
            return;
        }
    }
    
    //  Set up the WiFi interface and start DHCP process
    esp_netif_action_connected(s_wifi_netif, 
                                event_base, 
                                event_id, 
                                event_data);
    
    // Set wifi connected bit
    xEventGroupSetBits (e_wifi_event_group, WIFI_STA_CONNECTED_BIT);
}

static void wifi_disconnected_cb(){
    EventBits_t chosen = xEventGroupGetBits (e_wifi_event_group);
    if (chosen & WIFI_STA_STOP){
        // User call stop function                
        ESP_LOGI (TAG, "Diconnect is caused by stop function");
    }
    else if (chosen & WIFI_STA_DISCONNECT){
        // User call disconnect function                
        ESP_LOGI (TAG, "Disconnect successfully from %s", CONFIG_WIFI_STA_SSID);
    }
    else{
        // Connect failed from esp_connect function
        ESP_LOGE (TAG, "Connect failed to %s", CONFIG_WIFI_STA_SSID);
        if (chosen & WIFI_CHOSEN_STA_RECONENCT && reconnect_count < 6){
            // Reconnect is chosen                    
            reconnect_wifi();
        }
        else{
            // Reconenct is not chosen or run over reconnect count
            ESP_LOGE (TAG ,"Cannot connect to %s",CONFIG_WIFI_STA_SSID);
        }
    }
}

static void wifi_scan_done_cb(){
    xEventGroupSetBits (e_wifi_event_group,WIFI_STA_SCAN_DONE);
}

// Private support functions
/**
 * @brief
 */
static void reconnect_wifi(){
    esp_err_t esp_ret = esp_wifi_connect();
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failled to reconnect to %s",CONFIG_WIFI_STA_SSID);
    }
    ESP_LOGI (TAG, "Reconnect to %s: %d",CONFIG_WIFI_STA_SSID,reconnect_count);
    reconnect_count++;
}
 


/*******************************************************************
 * Public function implement
 */

esp_err_t wifi_sta_init (EventGroupHandle_t event_group){
    esp_err_t esp_ret;
    ESP_LOGI (TAG, "Starting Wi-Fi in station mode...");

    // Save the event group handle
    if (event_group == NULL){
        ESP_LOGE (TAG, "Event group handle is not be initialize");
        return ESP_FAIL;
    }
    e_wifi_event_group = event_group;

    //  (s1.3) Create default WiFi network interface
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_WIFI_STA();
    s_wifi_netif = esp_netif_new(&netif_cfg);
    if (s_wifi_netif == NULL) {
        ESP_LOGE(TAG, "Failed to create WiFi network interface");
        return ESP_FAIL;
    } 
    
    // Create Netif driver
    s_wifi_netif_driver = (wifi_netif_driver_t *)esp_wifi_create_if_driver(WIFI_IF_STA);
    if (s_wifi_netif_driver == NULL) {
        ESP_LOGE(TAG, "Failed to create wifi interface handle");
        return ESP_FAIL;
    }

    //  Connect Netif driver to network interface
    esp_ret = esp_netif_attach(s_wifi_netif, s_wifi_netif_driver);
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach WiFi driver to network interface");
        return ESP_FAIL;
    }

    // Register wifi event
    esp_ret = esp_event_handler_register (WIFI_EVENT,
                                          ESP_EVENT_ANY_ID,
                                          &on_wifi_event,
                                          NULL);
    if (esp_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WiFi event handler");
        return ESP_FAIL;
    }

    // // Register IP event                                          
    // esp_ret = esp_event_handler_register (IP_EVENT,
    //                                       ESP_EVENT_ANY_ID,
    //                                       &on_ip_event,
    //                                       NULL);
    
    // if (esp_ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to register IP event handler");
    //     return ESP_FAIL;
    // }
    
    // Initialize Wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_ret = esp_wifi_init (&cfg);
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to initialize WiFi");
        return ESP_FAIL;
    }
    // Set wifi mode to station
    esp_ret = esp_wifi_set_mode (WIFI_MODE_STA);
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to set Wifi mode to station");
        return ESP_FAIL;
    }

    // (2) Configure WiFi connection
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_STA_SSID,
            .password = CONFIG_WIFI_STA_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH
        }
    };
    esp_ret = esp_wifi_set_config (WIFI_IF_STA, &wifi_config);
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to set wifi configuration");
        return ESP_FAIL;
    }

    // (3.1) Start the WiFi driver
    esp_ret = esp_wifi_start();
    if (esp_ret != ESP_OK){
        ESP_LOGE (TAG, "Failed to start the WiFi driver");
        return ESP_FAIL;
    }
    return ESP_OK;
}    

esp_err_t wifi_sta_stop (void){
    xEventGroupSetBits (e_wifi_event_group,WIFI_STA_STOP);
    return esp_wifi_stop();
}

