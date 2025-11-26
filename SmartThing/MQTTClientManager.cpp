#include "MQTTClientManager.h"

MQTTClientManager::MQTTClientManager(WiFiClientSecure& wifiClient, const char* broker, int port, const char* clientId)
  : wifiClient(wifiClient), client(wifiClient), broker(broker), port(port), clientId(clientId), callback(nullptr),
    lastReconnectAttempt(0), lastKeepAlive(0), isReconnecting(false), reconnectAttempts(0) {
  client.setServer(broker, port);
}

void MQTTClientManager::setCallback(void (*cb)(char*, byte*, unsigned int)) {
  callback = cb;
  client.setCallback(callback);
}

void MQTTClientManager::reconnect() {
  unsigned long now = millis();
  if (client.connected()) {
    return;  // Ya conectado
  }
  //if (now - lastReconnectAttempt < RECONNECT_INTERVAL) {
  //  return;  // Esperar el intervalo para reintento
  //}
  Serial.print("Intentando reconectar MQTT...");
  if (client.connect(clientId)) {
    Serial.println("conectado");
    reconnectAttempts = 0;
  } else {
    Serial.print("fallo, rc=");
    Serial.print(client.state());
    Serial.println(". Intentar de nuevo en 5 segundos.");
    reconnectAttempts++;
  }
  lastReconnectAttempt = now;
}

void MQTTClientManager::subscribe(const char* topic) {
  if (client.subscribe(topic)) {
    Serial.println("Subscrito a " + String(topic));
  } else {
    Serial.println("Fallo al subscribirse a " + String(topic));
  }
}

bool MQTTClientManager::publish(const char* topic, const char* payload) {
  bool result = client.publish(topic, payload);
  if (result) {
    Serial.println("Mensaje publicado en " + String(topic));
  } else {
    Serial.println("Fallo al publicar en " + String(topic));
  }
  return result;
}

void MQTTClientManager::loop() {
  client.loop();
}

bool MQTTClientManager::isConnected() {
  return client.connected();
}

void MQTTClientManager::setServer(const char* broker, int port) {
  client.setServer(broker, port);
}

void MQTTClientManager::disconnect() {
  if (client.connected())
    client.disconnect();
}
