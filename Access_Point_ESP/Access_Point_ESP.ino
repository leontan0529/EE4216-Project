#include <WiFi.h>
#include "ESP32MQTTClient.h"

// Network credentials
const char* ssid = "ESP32-AP";
const char* password = "12345678";

// Static IP configuration
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// MQTT details
const char* mqttServer = "mqtt://192.168.4.3:1883";
ESP32MQTTClient mqttClient; // all params are set later

void setup() {
  Serial.begin(115200);

  // Set Wi-Fi mode to Access Point
  WiFi.mode(WIFI_AP);

  // Configure the static IP address
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("Failed to configure static IP address");
  }
  
  // Configure the access point
  WiFi.softAP(ssid, password);

  // Print the IP address of the access point
  Serial.print("Access Point IP address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // Nothing to do here
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        // For MQTT subscribe
    }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}