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

char *alarmTopic = "img";
char *clearAlarmTopic = "deactivate";
char *tempTopic = "tem";       // Topic for temperature
char *humidTopic = "hum"; // Topic for humidity
char *gasTopic = "gas";         // Topic for gas concentration
char *lightTopic = "lux";     // Topic for light level

ESP32MQTTClient mqttClient; // all params are set later

// Set global flags
bool alarmActivated = false;
bool tempReceived = false;
bool humidReceived = false;
bool gasReceived = false;
bool lightReceived = false;

// Containers to hold values from readings
String temp;
String humid;
String gas;
String light;

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
  if (alarmActivated) {
    
      // Do not clear the alarm until a signal is received
      while (alarmActivated)
      {
        delay(10);
      }
    
  }
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe(alarmTopic, [](const String &payload)
            {
              if (String(payload.c_str()) == "1") {
                alarmActivated = true;
              }
            });

        mqttClient.subscribe(clearAlarmTopic, [](const String &payload)
            {
              if (String(payload.c_str()) == "1") {
                alarmActivated = false;
              }
            });

        mqttClient.subscribe(tempTopic, [](const String &payload)
            {
              tempReceived = true;
              temp = String(payload.c_str());
            });

        // Subscribe to humidity topic
        mqttClient.subscribe(humidTopic, [](const String &payload)
          {
              humidReceived = true;
              humid = String(payload.c_str()); // Assign payload to humid
          });

        // Subscribe to gas concentration topic
        mqttClient.subscribe(gasTopic, [](const String &payload)
          {
              gasReceived = true;
              gas = String(payload.c_str()); // Assign payload to gas
          });

        // Subscribe to light level topic
        mqttClient.subscribe(lightTopic, [](const String &payload)
          {
              lightReceived = true;
              light = String(payload.c_str()); // Assign payload to light
          });
    }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}