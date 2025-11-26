#pragma once

#include <WiFi.h>

class WiFiManager {
private:
  const char* ssid;
  const char* password;
public:
  WiFiManager(const char* ssid, const char* password);
  void connect();
  bool isConnected();
};
