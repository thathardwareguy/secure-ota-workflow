// httpconnect.h
#ifndef HTTPCONNECT_H
#define HTTPCONNECT_H

#include "esp_http_client.h"

char *http_response_data;
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt);
char *extractURLFromResponse(const char *jsonResponse);

#endif // HTTPCONNECT_H