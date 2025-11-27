#include "MQTTClientManager.h"

// Constructor: Initializes MQTT client with WiFi connection and broker settings
MQTTClientManager::MQTTClientManager(WiFiClientSecure& wifiClient, const char* broker, int port, const char* clientId)
  : wifiClient(wifiClient), client(wifiClient), broker(broker), port(port), clientId(clientId), callback(nullptr),
    lastReconnectAttempt(0), lastKeepAlive(0), isReconnecting(false), reconnectAttempts(0) {
  client.setServer(broker, port);  // Configure MQTT broker address and port
}

// Register callback function to handle incoming messages from subscribed topics
void MQTTClientManager::setCallback(void (*cb)(char*, byte*, unsigned int)) {
  callback = cb;
  client.setCallback(callback);  // Set callback in MQTT library
}

// Attempts to reconnect to MQTT broker if disconnected
void MQTTClientManager::reconnect() {
  unsigned long now = millis();
  
  // Skip if already connected
  if (client.connected()) {
    return;
  }
  
  Serial.print("Attempting MQTT reconnection...");
  
  if (client.connect(clientId)) {
    Serial.println("connected");
    reconnectAttempts = 0;
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());  // Print error code from MQTT library
    Serial.println(". Retrying in 5 seconds.");
    reconnectAttempts++;
  }
  
  lastReconnectAttempt = now;
}

// Subscribe to MQTT topic to receive messages
void MQTTClientManager::subscribe(const char* topic) {
  if (client.subscribe(topic)) {
    Serial.println("Subscribed to " + String(topic));
  } else {
    Serial.println("Failed to subscribe to " + String(topic));
  }
}

// Publish message to MQTT topic
bool MQTTClientManager::publish(const char* topic, const char* payload) {
  bool result = client.publish(topic, payload);
  
  if (result) {
    Serial.println("Message published to " + String(topic));
  } else {
    Serial.println("Failed to publish to " + String(topic));
  }
  
  return result;
}

// Process incoming messages and maintain MQTT connection - call in main loop
void MQTTClientManager::loop() {
  client.loop();
}

// Check if currently connected to MQTT broker
bool MQTTClientManager::isConnected() {
  return client.connected();
}

// Reconfigure MQTT broker settings
void MQTTClientManager::setServer(const char* broker, int port) {
  client.setServer(broker, port);
}

// Disconnect from MQTT broker
void MQTTClientManager::disconnect() {
  if (client.connected())
    client.disconnect();
}