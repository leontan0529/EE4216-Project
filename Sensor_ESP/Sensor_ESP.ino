#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include "DHT.h"
#include "BH1750.h"
#include "MQ2.h"

// DHT sensor setup
#define DHTPIN 4
#define DHTTYPE DHT22
#define MQ2_PIN 6
#define LIGHT_PIN 5

DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "khai_rizz";
const char *pass = "handsumboi";

// Assign static IP address to sensor ESP
IPAddress local_IP(192, 168, 157, 122);
IPAddress gateway(192, 168, 157, 138);
IPAddress subnet(255, 255, 255, 0);

char *server = "mqtt://192.168.157.167:1883";

char *subscribeTopic = "sensordata";
char *tempTopic = "tem";       // Topic for temperature
char *humidityTopic = "hum";   // Topic for humidity
char *gasTopic = "gas";        // Topic for gas concentration
char *lightTopic = "lux";      // Topic for light level

ESP32MQTTClient mqttClient;

// Task handles
TaskHandle_t mqttTaskHandle = NULL;
TaskHandle_t sensorTaskHandle = NULL;

// Function prototypes
void mqttTask(void *param);
void sensorTask(void *param);

void setup()
{
    Serial.begin(115200);
    log_i("setup, ESP.getSdkVersion(): ");
    log_i("%s", ESP.getSdkVersion());

    dht.begin();

    mqttClient.enableDebuggingMessages();
    mqttClient.setURI(server);
    mqttClient.enableLastWillMessage("lwt", "I am going offline");
    mqttClient.setKeepAlive(30);

    if (!WiFi.config(local_IP, gateway, subnet))
    {
        Serial.println("Failed to configure static IP!");
    }

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println("\nConnected to WiFi");
    WiFi.setHostname("sensor");

    mqttClient.loopStart();

    // Create FreeRTOS tasks
    xTaskCreatePinnedToCore(sensorTask, "Sensor Task", 4096, NULL, 1, &sensorTaskHandle, 1); // Run on core 1
    xTaskCreatePinnedToCore(mqttTask, "MQTT Task", 4096, NULL, 1, &mqttTaskHandle, 1);       // Run on core 1
}

void loop()
{
    // The main loop remains empty since tasks are running in FreeRTOS
    vTaskDelay(portMAX_DELAY);
}

// Task to handle MQTT
void mqttTask(void *param)
{
    for (;;)
    {
        mqttClient.loop();
        delay(10); // Let other tasks run
    }
}

// Task to handle sensor readings and MQTT publishing
void sensorTask(void *param)
{
    for (;;)
    {
        float temp = dht.readTemperature();
        float humidity = dht.readHumidity();
        float lightLevel = analogRead(LIGHT_PIN);
        float co = analogRead(MQ2_PIN);

        if (isnan(temp) || isnan(humidity))
        {
            Serial.println("Failed to read from DHT sensor!");
        }
        else
        {
            // Publish temperature and humidity readings
            mqttClient.publish(tempTopic, String(temp), 0, false);
            mqttClient.publish(humidityTopic, String(humidity), 0, false);
        }

        // Publish gas concentration
        mqttClient.publish(gasTopic, String(co), 0, false);

        // Publish light level reading
        mqttClient.publish(lightTopic, String(lightLevel), 0, false);

        // Log sensor readings
        Serial.print("Temperature: ");
        Serial.print(temp);
        Serial.print(" °C\tHumidity: ");
        Serial.print(humidity);
        Serial.print(" %\tGas Levels: ");
        Serial.print(co);
        Serial.print("\tLight Level: ");
        Serial.println(lightLevel);

        vTaskDelay(60000 / portTICK_PERIOD_MS); // Delay for 60 seconds
    }
}
