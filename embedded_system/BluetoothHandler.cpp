#include "esp32-hal.h"
#include <stdint.h>
#include "BluetoothHandler.h"
#include <BluetoothSerial.h>
#include <FS.h>
#include <SPIFFS.h>
#include "config.h"

BluetoothSerial SerialBT;
unsigned long bluetoothLastActivity = 0;
char receivedData[MAX_MESSAGE_LENGTH];

void setupBluetooth(uint8_t *btConnect) {
  SerialBT.begin("ESP32_BT_Device");
  Serial.println(F("Bluetooth Ready. Connect and Send Data."));

  if (!SPIFFS.begin(true)) {
    Serial.println(F("SPIFFS Mount Failed."));
    return;
  }
  bluetoothLastActivity = millis();
  *btConnect = 1;
}

void addCredentials(const char* username, const char* password) {
  File file = SPIFFS.open(WIFI_CREDS_FILE, FILE_WRITE);
  if (!file) {
    Serial.println(F("[!] Failed to Open File for Writing."));
    return;
  }
  file.print(username);
  file.print("%");
  file.println(password);
  file.flush();
  file.close();
}

void checkBluetooth(uint8_t *btConnect) {
  static uint8_t dataIndex = 0;

  if (*btConnect == 1 && (millis() - bluetoothLastActivity > MINUTE_TIMEOUT*3)) {
    Serial.println(F("[!] Bluetooth Timeout: No Activity Monitored.\n"));
    *btConnect = 0;
    dataIndex = 0;
    memset(receivedData, 0, sizeof(receivedData));
    SerialBT.end();
    return;
  }

  while (SerialBT.available() && *btConnect == 1) {
    bluetoothLastActivity = millis();
    char incomingChar = SerialBT.read();
    Serial.print(incomingChar);

    if (dataIndex < MAX_MESSAGE_LENGTH) {
      receivedData[dataIndex++] = incomingChar;
      receivedData[dataIndex] = '\0';
    } else {
      SerialBT.println(F("[!] Message too long, Discarded."));
      dataIndex = 0;
      *btConnect = 0;
      memset(receivedData, 0, sizeof(receivedData));
      return;
    }

    if (strstr(receivedData, "<destroy#>") != NULL) {
      SerialBT.println(F("[x] Session Ended Manually."));
      dataIndex = 0;
      *btConnect = 0;
      delay(200);
      memset(receivedData, 0, sizeof(receivedData));
      SerialBT.end();
      return;
    }

    if (strstr(receivedData, "<end#>") != NULL) {
      if (strncmp(receivedData, SECRET_KEY, strlen(SECRET_KEY)) != 0) {
        SerialBT.println(F("[!] Invalid Secret Key. Data Rejected."));
        dataIndex = 0;
        memset(receivedData, 0, sizeof(receivedData));
        return;
      }

      char* dataPart = receivedData + strlen(SECRET_KEY);
      char* delimiter = strstr(dataPart, "<sep@>");
      if (delimiter == NULL) {
        SerialBT.println(F("[!] Invalid Format. Use: <KEY>username<sep@>password<end#>"));
        dataIndex = 0;
        memset(receivedData, 0, sizeof(receivedData));
        return;
      }

      *delimiter = '\0';
      char username[32] = {0};
      char password[64] = {0};
      strncpy(username, dataPart, sizeof(username) - 1);
      strncpy(password, delimiter + 6, sizeof(password) - 1);
      password[sizeof(password) - 1] = '\0';

      // Remove <end#> from the password if it exists
      char* endMarker = strstr(password, "<end#>");
      if (endMarker != nullptr) {
        *endMarker = '\0';
      }

      addCredentials(username, password);

      memset(username, 0, sizeof(username));
      memset(password, 0, sizeof(password));

      SerialBT.println(F("[✓] Credentials Stored Successfully. \nDo you want to Add More? Send data or `<destroy#>` command to Exit."));
      dataIndex = 0;
      memset(receivedData, 0, sizeof(receivedData));
      return;
    }
  }
}
