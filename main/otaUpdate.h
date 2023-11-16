// otaupdate.h
#ifndef OTAUPDATE_H
#define OTAUPDATE_H

#include "esp_https_ota.h"
#include "credentials.h"

esp_err_t validate_image_header(esp_app_desc_t *new_app_info);
void ota_task(const char *otaURL);

#endif // OTAUPDATE_H