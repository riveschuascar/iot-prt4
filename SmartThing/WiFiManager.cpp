#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password)
  : ssid(ssid), password(password) {}

void WiFiManager::connect() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi conectado. IP: ");
  Serial.println(WiFi.localIP());
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}