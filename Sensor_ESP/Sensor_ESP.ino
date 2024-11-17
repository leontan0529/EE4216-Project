#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include "DHT.h"
#include "BH1750.h"
#include "MQ2.h"

// DHT sensor setup
#define DHTPIN 4     
#define DHTTYPE DHT22

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60 /* Time ESP32 will go to sleep (in seconds) */

#define MQ2_PIN 6
#define LIGHT_PIN 5

DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "khai_rizz";
const char *pass = "handsumboi";

// Assign static IP address to sensor ESP
IPAddress local_IP(192, 168, 157, 122);
IPAddress gateway(192, 168, 157, 138);
IPAddress subnet(255, 255, 255, 0);

// Test Mosquitto server, see: https://test.mosquitto.org
char *server = "mqtt://192.168.157.167:1883";

char *subscribeTopic = "sensordata";
//char *publishTopic = "hello/esp";
char *tempTopic = "tem";       // Topic for temperature
char *humidityTopic = "hum"; // Topic for humidity
char *gasTopic = "gas";         // Topic for gas concentration
char *lightTopic = "lux";     // Topic for light level

ESP32MQTTClient mqttClient;

void setup()
{
    Serial.begin(115200);
    log_i();
    log_i("setup, ESP.getSdkVersion(): ");
    log_i("%s", ESP.getSdkVersion());

    dht.begin();
    // pinMode(MQ2_PIN, INPUT);

    mqttClient.enableDebuggingMessages();

    mqttClient.setURI(server);
    mqttClient.enableLastWillMessage("lwt", "I am going offline");
    mqttClient.setKeepAlive(30);
	
	// Configure the ESP32 to use a static IP
	if (!WiFi.config(local_IP, gateway, subnet)) {
	  Serial.println("Failed to configure static IP!");
	}
	
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
    }
    Serial.println("\nConnected to WiFi");
    WiFi.setHostname("sensor");
    mqttClient.loopStart();
}

int pubCount = 0;


void loop()
{
    /*
    String msg = "Hello: " + String(pubCount++);
    mqttClient.publish(publishTopic, msg, 0, false);
    */
    float temp = dht.readTemperature();    
    float humidity = dht.readHumidity();   
    float lightLevel = analogRead(LIGHT_PIN);

    float co = analogRead(MQ2_PIN);             // Read CO concentration


    if (isnan(temp) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

 // Publish temperature and humidity readings
    mqttClient.publish(tempTopic, String(temp), 0, false);
    mqttClient.publish(humidityTopic, String(humidity), 0, false);
    
    // Publish gas concentration
    mqttClient.publish(gasTopic, String(co), 0, false);

    // Publish light level reading
    mqttClient.publish(lightTopic, String(lightLevel), 0, false);

    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print(" °C\tHumidity: ");
    Serial.print(humidity);
    Serial.print(" %\tGas Levels: ");
    Serial.print(co);
    Serial.print("\tLight Level: ");
    Serial.println(lightLevel);

    // Sleep, then wake up every 60s to take and publish readings
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Entering deep sleep mode...");

    Serial.flush();
    esp_deep_sleep_start();
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe(subscribeTopic, [](const String &payload)
                             { Serial.println(String(subscribeTopic)+String(" ")+String(payload.c_str())); });

        mqttClient.subscribe("bar/#", [](const String &topic, const String &payload)
                             { log_i("%s: %s", topic, payload.c_str()); });
    }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}