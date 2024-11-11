#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include "DHT.h"
#include "BH1750.h"
#include "MQ2.h"

// DHT sensor setup
#define DHTPIN 4     
#define DHTTYPE DHT22
#define MQ2_PIN A0

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightSensor;          // Create an instance of the BH1750 sensor
MQ2 mq2(MQ2_PIN);            // Create an instance of the MQ-2 sensor

const char *ssid = "ESP32-AP";
const char *pass = "12345678";

// Assign static IP address to cam
IPAddress local_IP(192, 168, 4, 4);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Test Mosquitto server, see: https://test.mosquitto.org
char *server = "mqtt://192.168.4.3:1883";

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
    lightSensor.begin(); // Initialize BH1750 sensor
    mq2.begin();         // Initialize MQ-2 sensor

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
    float lightLevel = lightSensor.readLightLevel();
    
    float co = mq2.readCO();               // Read CO concentration


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

    delay(60000); // Send data every 60 seconds
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