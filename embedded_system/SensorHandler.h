#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void setupSensors(uint8_t *sensorConnect);
float readDHTTemperature();
float readDHTHumidity();
void takeSensorReading(float *temperature, float *humidity);
void sendDataToSupabase(float temperature, float humidity);

#endif