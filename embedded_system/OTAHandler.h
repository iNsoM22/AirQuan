#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

void checkForNewUpdate();
void updateFirmware(const char* updateFileUrl);

#endif
