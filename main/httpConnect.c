// httpconnect.c
#include "httpConnect.h"
#include "esp_http_client.h"
#include "credentials.h"
#include "esp_log.h"
#include "stdlib.h"

static const char *TAG = "HTTP-CONNECT";

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "Client HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
            if (http_response_data == NULL) {
                http_response_data = (char *)malloc(evt->data_len + 1);
                if (http_response_data != NULL) {
                    strncpy(http_response_data, (char *)evt->data, evt->data_len);
                    http_response_data[evt->data_len] = '\0';
                }
            } else {
                char *new_data = (char *)realloc(http_response_data, strlen(http_response_data) + evt->data_len + 1);
                if (new_data != NULL) {
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

char *extractURLFromResponse(const char *jsonResponse) {
    const char *responseKey = "\"Response\":\"";
    const char *responseStart = strstr(jsonResponse, responseKey);
    if (responseStart) {
        responseStart += strlen(responseKey);
        const char *responseEnd = strchr(responseStart, '"');
        if (responseEnd) {
            size_t urlLength = responseEnd - responseStart;
            char *url = (char *)malloc(urlLength + 1);
            if (url) {
                strncpy(url, responseStart, urlLength);
                url[urlLength] = '\0';
                if (url[0] == '"' && url[urlLength - 1] == '"') {
                    memmove(url, url + 1, urlLength - 1);
                    url[urlLength - 1] = '\0';
                }
                return url;
            }
        }
    }
    return NULL;
}