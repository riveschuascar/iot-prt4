#pragma once

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino.h>

class MQTTClientManager {
private:
  WiFiClientSecure& wifiClient;
  PubSubClient client;
  const char* broker;
  int port;
  const char* clientId;
  void (*callback)(char*, byte*, unsigned int);

  unsigned long lastReconnectAttempt;
  unsigned long lastKeepAlive;
  static const unsigned long RECONNECT_INTERVAL = 5000;
  static const unsigned long KEEPALIVE_INTERVAL = 30000;

  bool isReconnecting;
  int reconnectAttempts;
  static const int MAX_RECONNECT_ATTEMPTS = 5;

public:
  MQTTClientManager(WiFiClientSecure& wifiClient, const char* broker, int port, const char* clientId);
  void setCallback(void (*callback)(char*, byte*, unsigned int));
  void reconnect();
  void subscribe(const char* topic);
  bool publish(const char* topic, const char* payload);
  void loop();
  bool isConnected();
  void setServer(const char* broker, int port);
  void handleReconnection();
  void sendKeepAlive();
  int getReconnectAttempts();
  void resetReconnectAttempts();
  void disconnect();
};