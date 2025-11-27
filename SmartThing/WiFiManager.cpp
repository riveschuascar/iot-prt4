#include "WiFiManager.h"

// Constructor: Store WiFi credentials
WiFiManager::WiFiManager(const char* ssid, const char* password)
  : ssid(ssid), password(password) {}

// Connect to WiFi network - blocks until connection established
void WiFiManager::connect() {
  delay(10);  // Small delay to stabilize before WiFi initialization
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);  // Start WiFi connection attempt
  
  // Block until connected (WL_CONNECTED status)
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);      // Check connection every 500ms
    Serial.print("."); // Progress indicator
  }
  
  Serial.println();
  Serial.println("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());  // Print assigned IP address
}

// Check if WiFi is currently connected
bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}