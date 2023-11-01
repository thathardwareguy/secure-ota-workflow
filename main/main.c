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
#include "driver/gpio.h"
static const char *TAG = "OTA-UPDATE";
#define LED_GPIO 2 // Replace with the GPIO pin connected to your LED

// Global pointer to store the HTTP response
char *http_response_data = NULL;
// Define client certificate
extern const uint8_t ClientCert_pem_start[] asm("_binary_certificate_pem_start");
extern const uint8_t ClientCert_pem_end[] asm("_binary_certificate_pem_end");

// Your Wi-Fi connection code
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "WiFi connecting ...");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "WiFi connected ...");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "WiFi lost connection ...");
        break;
    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "WiFi got IP ...");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);

    esp_wifi_start();
    esp_wifi_connect();
}
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("Client HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);

        // Allocate or reallocate memory for the response and copy data
        if (http_response_data == NULL)
        {
            http_response_data = (char *)malloc(evt->data_len + 1);
            if (http_response_data != NULL)
            {
                strncpy(http_response_data, (char *)evt->data, evt->data_len);
                http_response_data[evt->data_len] = '\0';
            }
        }
        else
        {
            // Reallocate memory to accommodate the new data
            char *new_data = (char *)realloc(http_response_data, strlen(http_response_data) + evt->data_len + 1);
            if (new_data != NULL)
            {
                http_response_data = new_data;
                strncat(http_response_data, (char *)evt->data, evt->data_len);
            }
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}
static void client_post_rest_function()
{
    // Construct the complete URL with the current running version
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(esp_ota_get_running_partition(), &running_app_info) == ESP_OK)
    {
        char complete_url[256];
        snprintf(complete_url, sizeof(complete_url), "https://q4iyvnnu50.execute-api.us-east-2.amazonaws.com/dev/firmwares?rawVersion=%s", running_app_info.version);

        // Perform an HTTPS GET request
        esp_http_client_config_t config_get = {
            .url = complete_url,
            .method = HTTP_METHOD_GET,
            .cert_pem = (const char *)ClientCert_pem_start,
            .event_handler = client_event_get_handler};

        esp_http_client_handle_t client = esp_http_client_init(&config_get);
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "HTTPS GET request successful");
        }
        else
        {
            ESP_LOGE(TAG, "HTTPS GET request failed");
        }
        esp_http_client_cleanup(client);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to get the running firmware info");
    }
}

// Function to extract the URL from the JSON response
char *extractURLFromResponse(const char *jsonResponse)
{
    const char *responseKey = "\"Response\":\"";
    const char *responseStart = strstr(jsonResponse, responseKey);

    if (responseStart)
    {
        responseStart += strlen(responseKey);
        const char *responseEnd = strchr(responseStart, '"');

        if (responseEnd)
        {
            size_t urlLength = responseEnd - responseStart;
            char *url = (char *)malloc(urlLength + 2); // Allocate extra space for the double quotes

            if (url)
            {
                url[0] = '"';
                strncpy(url + 1, responseStart, urlLength);
                url[urlLength + 1] = '"';
                url[urlLength + 2] = '\0';
                return url;
            }
        }
    }

    return NULL;
}

static esp_err_t validate_image_header(esp_app_desc_t *new_app_info)
{
    if (new_app_info == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
    {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
    if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0)
    {
        ESP_LOGW(TAG, "Current running version is the same as the new one. OTA update is not required.");
        return ESP_FAIL;
    }
#endif

    return ESP_OK;
}

void ota_task(void *pvParameters)
{
    client_post_rest_function();
    // Check if the response has been captured and print it
    if (http_response_data != NULL)
    {

        // Extract the URL from the JSON response
        char *extractedURL = extractURLFromResponse(http_response_data);

        if (extractedURL)
        {
            printf("Extracted URL: %s\n", extractedURL);
    esp_err_t ota_finish_err = ESP_OK;
    esp_http_client_config_t config = {
        .url = extractedURL,
        .cert_pem = (const char *)ClientCert_pem_start,
        .timeout_ms = 5000,
        .keep_alive_enable = true,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
   
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
        vTaskDelete(NULL);
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
        goto ota_end;
    }
    err = validate_image_header(&app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Image header verification failed");
        goto ota_end;
    }

    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        // The OTA image was not completely received, handle this situation as needed.
        ESP_LOGE(TAG, "Complete data was not received.");
    } else {
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_restart();
        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            }
            ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
            vTaskDelete(NULL);
        }
    }

ota_end:
    esp_https_ota_abort(https_ota_handle);
    ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
    vTaskDelete(NULL);
            free(extractedURL);
        }

        free(http_response_data);
    }
}

void led_task(void *pvParameters)
{
    // Initialize the GPIO for the LED
    gpio_pad_select_gpio(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        // Toggle the LED state
        gpio_set_level(LED_GPIO, !gpio_get_level(LED_GPIO));

        // Print the state of the LED
        if (gpio_get_level(LED_GPIO)) {
            ESP_LOGI(TAG, "LED is ON");
        } else {
            ESP_LOGI(TAG, "LED is OFF");
        }

        vTaskDelay(pdMS_TO_TICKS(3000)); // 3-second interval
    }
}

void app_main(void)
{
    nvs_flash_init();
    wifi_connection();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n\n");

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("Start client:\n\n");

    // Create and start the OTA task
    xTaskCreate(ota_task, "ota_task", 4096, NULL, 5, NULL);

    // Create and start the LED task
    xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);
}
