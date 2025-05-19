#include "OTAHandler.h"
#include "config.h"
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>

void checkForNewUpdate() {
  HTTPClient http;

  // Construct Endpoint URL
  String url = String(SUPABASE_URL) + "/rest/v1/FirmwareManager?order=version.desc&limit=1&select=version,file_url";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_API_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_API_KEY);

  int httpResponseCode = http.GET();

  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print(F("[!] JSON Error: "));
      Serial.println(error.f_str());
      http.end();
      return;
    }

    JsonArray arr = doc.as<JsonArray>();
    if (arr.isNull() || arr.size() == 0) {
      Serial.println(F("[!] No Firmware Record Found."));
      http.end();
      return;
    }
    // Get version as float (not const char*)
    float newVersion = arr[0]["version"];
    const char* updateUrl = arr[0]["file_url"];

    if (updateUrl == nullptr || strlen(updateUrl) == 0) {
      Serial.println(F("[!] No update URL found!"));
      http.end();
      return;
    }
    Serial.print(F("New Version: "));
    Serial.println(newVersion, 2);
    
    if (newVersion > CURRENT_FIRMWARE_VERSION) {
      Serial.println(F("[OTA] Update Available. Starting..."));
      updateFirmware(updateUrl);
    } else {
      Serial.println(F("[OTA] Firmware is Up to Date."));
    }

    doc.clear();
  } else {
    Serial.print(F("[!] HTTP Error: "));
    Serial.println(httpResponseCode);
    Serial.print(F("[!] Response: "));
    Serial.println(http.getString());
  }

  http.end();
}

void updateFirmware(const char* updateFileUrl) {
  if (updateFileUrl == nullptr || strlen(updateFileUrl) == 0) {
    Serial.println(F("[!] Invalid Firmware URL. Aborting Update."));
    return;
  }

  HTTPClient http;
  http.begin(updateFileUrl);
  http.addHeader("apikey", SUPABASE_API_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_API_KEY);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient& client = http.getStream();
    uint32_t contentLength = http.getSize();

    if (contentLength == 0) {
      Serial.println(F("[!] Firmware File is Empty."));
      http.end();
      return;
    }

    if (!Update.begin(contentLength)) {
      Serial.println(F("[!] Insufficient Space for OTA."));
      http.end();
      return;
    }

    size_t written = Update.writeStream(client);

    if (written == contentLength) {
      Serial.println(F("[✓] Firmware Written Successfully."));
    } else {
      Serial.print(F("[!] Incomplete Firmware Update. Bytes written: "));
      Serial.println(written);
    }

    if (Update.end()) {
      if (Update.isFinished()) {
        Serial.println(F("[✓] OTA Complete. Restarting..."));
        delay(2000);
        ESP.restart();
      } else {
        Serial.println(F("[!] OTA Not Finished Properly."));
      }
    } else {
      Serial.print(F("[!] OTA Failed."));
    }
  } else {
    Serial.print(F("[!] Firmware Download Failed. HTTP code: "));
    Serial.println(httpCode);
  }
  http.end();
}
