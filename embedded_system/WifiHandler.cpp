#include <sys/types.h>
#include "WifiHandler.h"
#include "WiFi.h"
#include <FS.h>
#include <SPIFFS.h>
#include "config.h"

void connectToWiFiFromFile(uint8_t *wifiConnect) {
  File file = SPIFFS.open(WIFI_CREDS_FILE, FILE_READ);
  delay(500);
  
  if (!file || file.size() == 0) {
    Serial.println(F("[!] Failed to Open Credentials File or File is Empty."));
    return;
  }

  String credsLine;
  while (file.available()) {
    credsLine = file.readStringUntil('\n');  // Read the last line (in case multiple lines exist)
  }
  file.close();
  delay(500);

  credsLine.trim();  // Removes leading/trailing spaces/newlines

  int separatorIndex = credsLine.indexOf('%');
  if (separatorIndex == -1) {
    Serial.println(F("[!] Invalid credentials format. Expected 'SSID%PASSWORD'"));
    return;
  }

  String ssid = credsLine.substring(0, separatorIndex);
  String password = credsLine.substring(separatorIndex + 1);

  ssid.trim();
  password.trim();

  WiFi.mode(WIFI_STA);

  Serial.print(F("[*] SSID: \"")); Serial.print(ssid); Serial.println("\"");
  Serial.print(F("[*] PASS: \"")); Serial.print(password); Serial.println("\"");

  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 15000;

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(1000);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\n[✓] WiFi Connected!"));
    Serial.print(F("[✓] IP Address: "));
    Serial.println(WiFi.localIP());
    *wifiConnect = 0;
  } else {
    Serial.println(F("\n[!] Failed to connect to WiFi."));
  }
}
