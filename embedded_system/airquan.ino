#include <Arduino.h>
#include "config.h"
#include "BluetoothHandler.h"
#include "WifiHandler.h"
#include "SensorHandler.h"
#include "OTAHandler.h"

uint8_t btConnect = 0;
uint8_t wifiConnect = 1;
uint8_t sensorConnect = 0;
unsigned long lastDataSendTime = 0;
uint8_t dataSendCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("=== DEVICE BOOT ==="));
  Serial.printf("Reset Reason: %d\n", esp_reset_reason());

  Serial.print(F("[OTA] Current Version: "));
  Serial.println(CURRENT_FIRMWARE_VERSION);

  setupBluetooth(&btConnect);
  delay(2000);

  setupSensors(&sensorConnect);
  delay(500);
}

void loop() {

  // Bluetooth Mode for 180 Seconds.
  if(btConnect == 1) {
    checkBluetooth(&btConnect);
    delay(1000);
    return;
  }

  // Then, Connect to the Wifi
  if(wifiConnect == 1 && btConnect == 0){
    connectToWiFiFromFile(&wifiConnect);
    delay(2000);
    return;
    }

  // Check if a New Update is Available (30 Mints Interval)
  if(dataSendCounter >= 2){
    checkForNewUpdate();
    delay(5000);
    dataSendCounter = 0;
  }

  // // Periodically Send Data to Supabase (every 1 minute)
  if (sensorConnect == 1 && ((millis() - lastDataSendTime) >= MINUTE_TIMEOUT)) {
    lastDataSendTime = millis();

    // Get Temperature and Humidity
    float temperature;
    float humidity;
    takeSensorReading(&temperature, &humidity);
    
    delay(500);
    // Send the Data to Supabase
    sendDataToSupabase(temperature, humidity);

    // Update the Counter
    dataSendCounter++;
  }
  delay(500);  
}
