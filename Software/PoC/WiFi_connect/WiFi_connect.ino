#include <WiFi.h>
#include "secret.h"

// Constants for Wi-Fi reconnection
const unsigned long CHECK_INTERVAL = 1000; // Check Wi-Fi every 1 second
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  connectToWiFi();
}

void loop() {
  unsigned long currentMillis = millis();

  // Check the Wi-Fi connection every second
  if (currentMillis - previousMillis >= CHECK_INTERVAL) {
    previousMillis = currentMillis;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wi-Fi lost. Reconnecting...");
      reconnectWiFi();
    } else {
      Serial.println("Wi-Fi connected. IP address: " + WiFi.localIP().toString());
    }
  }
}

void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi.");
    Serial.println("IP Address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
  }
}

void reconnectWiFi() {
  WiFi.disconnect();
  WiFi.reconnect();

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nReconnected to Wi-Fi.");
  } else {
    Serial.println("\nReconnection attempt failed.");
  }
}
