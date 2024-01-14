// otaupdate.c
#include "otaUpdate.h"
#include "esp_https_ota.h"
#include "credentials.h"
#include "esp_ota_ops.h"
#include "protocol_examples_common.h"
#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_log.h"
// Define client certificate
extern const uint8_t ClientCert_pem_start[] asm("_binary_certificate_pem_start");
extern const uint8_t ClientCert_pem_end[]   asm("_binary_certificate_pem_end");
static const char *TAG = "OTA-UPDATE";

// Function to handle HTTPS OTA update
static void perform_https_ota(const char *otaURL);

esp_err_t validate_image_header(esp_app_desc_t *new_app_info) {
    if (new_app_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
    if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0) {
        ESP_LOGW(TAG, "Current running version is the same as the new one. OTA update is not required.");
        return ESP_FAIL;
    }
#endif

    return ESP_OK;
}

void ota_task(const char *otaURL) {
    printf("OTA URL: %s\n", otaURL);

    // Check if the provided OTA URL contains "https://"
    if (strstr(otaURL, "https://") == NULL) {
        ESP_LOGE(TAG, "Response does not contain 'https://': %s", otaURL);
        printf("RES: Update done\n");
    } else {
        // Perform the HTTPS OTA update
        perform_https_ota(otaURL);
    }
}

// Function to handle HTTPS OTA update
static void perform_https_ota(const char *otaURL) {
    esp_err_t ota_finish_err = ESP_OK;

    // Configure HTTPS client
    esp_http_client_config_t config = {
        .url = otaURL,
        .cert_pem = (const char *)ClientCert_pem_start,
        .timeout_ms = 20000,
        .keep_alive_enable = true,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    // Begin HTTPS OTA update
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");

        // Terminate the task if the update begins with an error
        vTaskDelete(NULL);
    }

    esp_app_desc_t app_desc;
    // Get image description
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");

        // Cleanup and abort the OTA update
        goto ota_end;
    }

    // Validate image header
    err = validate_image_header(&app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Image header verification failed");

        // Cleanup and abort the OTA update
        goto ota_end;
    }

    // Perform OTA update
    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
    }

    // Check if complete data is received
    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        ESP_LOGE(TAG, "Complete data was not received.");
    } else {
        // Finish OTA update
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");

            // Delay and restart the system
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_restart();
        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                ESP_LOGE(TAG, "Image validation failed, image is corrupted");
            }
            ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);

            // Terminate the task in case of update failure
            vTaskDelete(NULL);
        }
    }

ota_end:
    // Abort HTTPS OTA update
    esp_https_ota_abort(https_ota_handle);
    ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
}
