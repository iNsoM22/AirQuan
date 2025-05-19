#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <Arduino.h>

void setupBluetooth(uint8_t *btConnect);
void checkBluetooth(uint8_t *btConnect);
void addCredentials(const char* username, const char* password);

#endif