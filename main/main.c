#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "credentials.h"
#include "protocol_examples_common.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include <esp_event.h>
#include <esp_system.h>
#include "sdkconfig.h"
#include "httpconnect.h"
#include "otaupdate.h"
#include "wificonnect.h"
// Define client certificate
extern const uint8_t ClientCert_pem_start[] asm("_binary_certificate_pem_start");
extern const uint8_t ClientCert_pem_end[]   asm("_binary_certificate_pem_end");
static const char *TAG = "OTA-UPDATE";

static void client_post_rest_function() {
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(esp_ota_get_running_partition(), &running_app_info) == ESP_OK) {
        char complete_url[256];
        snprintf(complete_url, sizeof(complete_url), "https://stgquzvr3h.execute-api.us-east-2.amazonaws.com/dev/firmwares?rawVersion=%s", running_app_info.version);

        esp_http_client_config_t config_get = {
            .url = complete_url,
            .method = HTTP_METHOD_GET,
            .cert_pem = (const char *)ClientCert_pem_start,
            .event_handler = client_event_get_handler
        };

        esp_http_client_handle_t client = esp_http_client_init(&config_get);
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "HTTPS GET request successful");
        } else {
            ESP_LOGE(TAG, "HTTPS GET request failed");
        }
        esp_http_client_cleanup(client);
    } else {
        ESP_LOGE(TAG, "Failed to get the running firmware info");
    }
}

void app_main(void) {
    nvs_flash_init();
    wifi_connection();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n\n");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("Start client:\n\n");
    client_post_rest_function();

    if (http_response_data != NULL) {
        char *extractedURL = extractURLFromResponse(http_response_data);

        if (extractedURL) {
            printf("Extracted URL: %s\n", extractedURL);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            ota_task(extractedURL);
            free(extractedURL);
        }

        free(http_response_data);
    }
}