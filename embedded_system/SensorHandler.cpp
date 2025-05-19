#include "SensorHandler.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "config.h"

#define DHTPIN 27
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Setup DHT sensor
void setupSensors(uint8_t *sensorConnect) {
  dht.begin();
  delay(2000); 
  pinMode(DHTPIN, INPUT);
  *sensorConnect = 1;
}

// Read Temperature
float readDHTTemperature() {
  float t = dht.readTemperature();
  delay(500);
  if (isnan(t)) {
    Serial.println(F("Failed to Read Temperature from DHT!"));
    return NULL;
  }
  Serial.println("Temperature: " + String(t) + " °C");
  return t;
}

// Read Humidity
float readDHTHumidity() {
  float h = dht.readHumidity();
  delay(500);
  if (isnan(h)) {
    Serial.println(F("Failed to Read Humidity from DHT!"));
    return NULL;
  }
  Serial.println("Humidity: " + String(h) + " %");
  return h;
}

// Store Readings into the Array
void takeSensorReading(float *temperature, float *humidity) {
  delay(1500);
  *temperature = readDHTTemperature();
  delay(1500);
  *humidity = readDHTHumidity();

  if (isnan(*temperature) || isnan(*humidity)) {
    Serial.println(F("[!] Failed to Read from DHT Sensor."));
    return;
  } 
}

// Upload the Data to Supabase
void sendDataToSupabase(float temperature, float humidity) {
  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/SensorData";

  // Prepare the JSON Payload
  DynamicJsonDocument doc(512);
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  
  String payload;
  serializeJson(doc, payload);

  // Prepare Headers for the POST Request
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_API_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_API_KEY);

  // Send the POST Request to Insert Data into the Table
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode == 201) {
    Serial.println(F("[✓] Data Successfully Sent to Supabase."));
  } else {
    Serial.print(F("[!] Failed to Send Data. Response code: "));
    Serial.println(httpResponseCode);
  }
  http.end();
}
