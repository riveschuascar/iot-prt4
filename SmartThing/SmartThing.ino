#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "MQTTClientManager.h"
#include "WiFiManager.h"
#include "GasSensor.h"
#include "GateServo.h"

#define GAS_SENSOR_PIN 34
#define SERVO_PIN 25

const char* WIFI_SSID = "TIGO RIVERO";
const char* WIFI_PASS = "a36bb1e335c28";
const char* MQTT_BROKER = "a1n3h8klbo6gqj-ats.iot.us-west-1.amazonaws.com";
const int MQTT_PORT = 8883;
const char* CLIENT_ID = "ESmarthP-32";

const char* UPDATE_TOPIC = "$aws/things/esp32-maqueta/shadow/update";
const char* UPDATE_DELTA_TOPIC = "$aws/things/esp32-maqueta/shadow/update/delta";

const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

const char CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUZpXzDjUo+dR+GcgP9HlDs2G8eUIwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTExNjIxNDMx
MVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJzLieqAPt5jhOVGxRX+
TRHLeNCZw6nuNwUgTeb4Eni44nQNhI6UTNzlvhatiYDKr3jgj9L8VX0BtFB9YS+f
QVoD4npJznVguI1ATR/iFCiAA+q4CF7Xf7RMJN/z9l0r0XeOcdpGyHiPKxF9QWDA
b4tSFzvwFtGruhwQqL6Bg0Vkg26rgr8UrO0Qk3/OHeAes+hXeS/HhjzS1PC0em9r
o894ITm6vQMBW+UhhHtlKpKQko4CH9L06cEjFDXHyv94ZMiZBcj8GOBMLJMU68jO
EQ/Kva9H/PXpqSZIBKt/D8YCm0aTgvkEUwW3zXb2Zhrv10LxyG5Kmgtola0O8pTQ
qZMCAwEAAaNgMF4wHwYDVR0jBBgwFoAUOGL62cfkpnbEXnhLYZKf4Aqq1VswHQYD
VR0OBBYEFL0NOjpORs7wwCN45uDzMLHxVn8nMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAn3PZxzAEDEdROA9B9CnPgwR+7
HNd8YfQID8lMehKKzxOzt2CEgbn6zizvHrm9HnLjD0GTusQmaCS/+q+MCqyJC5Tn
xnQQpvB2YGat58tqnO8ZXn/T8roNQ3o9eJTdaPNT5JuICZU43nY8g2+IFT9Bifm0
jnJDC2hx1GblSv31siFudabXPNiSw7LFJPa2pf9hFA9o/1piK2QtiSuYaZNApSfg
cmRVlkeuPO/6oG8Y2Ybvul1JF5KuADws7yF4gtOHmGdcghdgeLsdTQeWHTxbN4cC
Te1xPqeGKegnnsI9KCXtF297IPdsfOgG5/32MTX9T6FDG2/4ZQOJAhmYi9Ip
-----END CERTIFICATE-----
)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAnMuJ6oA+3mOE5UbFFf5NEct40JnDqe43BSBN5vgSeLjidA2E
jpRM3OW+Fq2JgMqveOCP0vxVfQG0UH1hL59BWgPieknOdWC4jUBNH+IUKIAD6rgI
Xtd/tEwk3/P2XSvRd45x2kbIeI8rEX1BYMBvi1IXO/AW0au6HBCovoGDRWSDbquC
vxSs7RCTf84d4B6z6Fd5L8eGPNLU8LR6b2ujz3ghObq9AwFb5SGEe2UqkpCSjgIf
0vTpwSMUNcfK/3hkyJkFyPwY4EwskxTryM4RD8q9r0f89empJkgEq38PxgKbRpOC
+QRTBbfNdvZmGu/XQvHIbkqaC2iVrQ7ylNCpkwIDAQABAoIBAB5nSWLzYz10Rry+
sFDHcuJUdiTc7BfzXS/dR73VtJ88Go0uYdnT3+OlmOqE0/jpVIU35iufSmRnBDPn
XAUD+IQf5LaZwTL5gk/BfDMGf6oqyJem/9iEKPspGK1kfQ0sBf4M3P7aYt0dxdgq
VZcROY/mZyL0NziI+BmhEVd/fZjiyu6/KoJrN5NBj20SdGDDjhM1sh+bDpLaf4LY
0YyjMj6P3TOP9CAaxVX+3aVMVWL+eGZUUtPPhZ+WX1h8GQnlantWYjDoQ/MAyIkY
oBffFxxAxkjTqndMzI2Oe9w1hY3S4lD0kscbXxn0VaUbYZGBPF1bu5waNH97+zeS
GndOaWECgYEAzhuUGgd5v3o9OBy82mXB/b7tOjBJ7oZxH5IlNQAb9JDXJdWIzCZp
DvQitlzhMIoyOYRMLawKHv5nk1L/uraM5WL5GVwqaj9eSj83eDsvjdUqonbTbMT3
7jDIAuKIqOdlOFJC+Ya0zRXYC/zJQdwuyjNJ0QGQ9Bk9QvLpzXOSC5cCgYEAwsAS
oSvYnGqqC+IFtihqHHEN9MIlZSabWwNWX9XfnxfoioLz+1X4KSYXyrtIX+X20NfR
Ucv5/huH36uD8YtEh65zab1t6GsrDY+NAmj43MQD5I47m6d9swPILWeYVJdfvZvx
DlP4NiMNnQbELHJErgGNhFxpYRjrDBF3YBXdgWUCgYAdyd6d5Ha0NfajJYQmDI79
HZsrzc7hqoyvA3BdOKbRsh1mWnZrCyIkByT9Nm9VlKOtrHFWKIdN83cE0/oiAkBa
5vMZtzhqIr59/KUHu4Yj+asvz/y+u7kZs/M0d4lI4CjA+yKy7cUz3vRaxy5PVoNs
tfz1OES7AurXwkQIbauuAQKBgCLy9yTnSnMfjS6evCmpbQl3nGqNIMrbN8wIeEqw
hfX2A2w//erfza4MwP5Hx8A186oc4NqOAWoBpMuV6xLAzGaQ/vM95GhuOwau+T4y
el6b6prsj3PqhdtHVXgYfDTKCYtsXcEVfpiwh7PjT/ct9ndHng7fyqV7JEtBIywi
IGMlAoGBAISAEoKJA/BY7WL8EWyJp91sELHVqWy8j4e182fpMS518XfIsRnb49mw
SH7eHu1TIdB7m24yHCoxKs8fo//s1ZzWZlx8u3wehEkg9aucXvwhNilZ9Ex4RYAA
yDX8L/izuNHFYM/SMDbVo73aTU/oRAUODU9vvHN1YM8soQ3OzE5U
-----END RSA PRIVATE KEY-----
)KEY";

// -------------------
// Variables globales
// -------------------
WiFiManager wifiManager(WIFI_SSID, WIFI_PASS);
WiFiClientSecure wiFiClient;
MQTTClientManager mqttClient(wiFiClient, MQTT_BROKER, MQTT_PORT, CLIENT_ID);
GasSensor gasSensor(GAS_SENSOR_PIN, 5.0, 9.83);
GateServo gate(SERVO_PIN);

String lastGasLevelState = "";

StaticJsonDocument<512> inputDoc;
StaticJsonDocument<512> outputDoc;
char outputBuffer[512];

// -------------------
// Reportar estados
// -------------------
void reportStates() {
  if (!mqttClient.isConnected()) {
    mqttClient.reconnect();
    mqttClient.subscribe(UPDATE_DELTA_TOPIC);
  }
  
  outputDoc.clear();
  outputDoc["state"]["reported"]["gasLevel_ppm"] = gasSensor.getPPM();
  outputDoc["state"]["reported"]["gasLevel_state"] = gasSensor.getGasLevel();
  outputDoc["state"]["reported"]["gate_state"] = gate.getState();
  
  serializeJson(outputDoc, outputBuffer);
  Serial.println(">>> Publicando a IoT Core:");
  Serial.println(outputBuffer);
  mqttClient.publish(UPDATE_TOPIC, outputBuffer);
}

// -------------------
// Callback MQTT
// -------------------
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
  Serial.println("Mensaje recibido en: " + String(topic) + " => " + message);
  
  inputDoc.clear();
  DeserializationError err = deserializeJson(inputDoc, payload, length);
  if (err) {
    Serial.println("ERROR parseando JSON: " + String(err.c_str()));
    return;
  }
  
  if (String(topic) == UPDATE_DELTA_TOPIC) {
    JsonObject state = inputDoc["state"];
    if (state.containsKey("gate_state")) {
      String desiredValve = state["gate_state"].as<String>();
      String currentValve = gate.getState();
      if (desiredValve != currentValve) {
        if (desiredValve == "open") gate.open();
        else gate.close();
        reportStates();
      }
    }
  }
}

// -------------------
// Setup
// -------------------
void setup() {
  delay(2000);
  Serial.begin(115200);
  Serial.println("\n\n=== Iniciando sistema ===");
  
  gate.begin();
  
  // CALIBRACIÓN BLOQUEANTE ANTES DE WIFI/MQTT
  Serial.println("=== CALIBRANDO SENSOR DE GAS ===");
  gasSensor.calibrateBlocking();
  Serial.println("=== CALIBRACIÓN COMPLETADA ===\n");
  
  // Ahora sí, conectar WiFi y MQTT
  wifiManager.connect();
  
  wiFiClient.setCACert(AMAZON_ROOT_CA1);
  wiFiClient.setCertificate(CERTIFICATE);
  wiFiClient.setPrivateKey(PRIVATE_KEY);
  
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
  mqttClient.reconnect();
  mqttClient.subscribe(UPDATE_DELTA_TOPIC);
  
  Serial.println("Sistema listo. Iniciando lecturas...\n");
}

// -------------------
// Loop principal
// -------------------
unsigned long lastReadingTime = 0;

void loop() {
  mqttClient.loop();
  
  // Iniciar nueva lectura cada 10 segundos
  if (millis() - lastReadingTime >= 10000) {
    lastReadingTime = millis();
    gasSensor.startReading();
  }
  
  // Actualizar lectura en progreso
  if (gasSensor.updateReading()) {
    // Lectura completada
    int currentPPM = gasSensor.getPPM();
    String currentGasState = gasSensor.getGasLevel();
    
    // Mostrar SIEMPRE en Serial
    Serial.print("[ ");
    Serial.print(millis() / 1000);
    Serial.print("s ] PPM: ");
    Serial.print(currentPPM);
    Serial.print(" | Estado: ");
    Serial.println(currentGasState);
    
    // Publicar SOLO cuando el ESTADO cambia
    if (currentGasState != lastGasLevelState) {
      Serial.println(">>> ¡CAMBIO DE ESTADO DETECTADO! Reportando a IoT Core...");
      lastGasLevelState = currentGasState;
      reportStates();
    }
  }
}
