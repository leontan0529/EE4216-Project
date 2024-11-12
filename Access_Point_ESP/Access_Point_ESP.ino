#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include <LiquidCrystal_I2C.h>
#include "custom_char.h"

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
volatile bool alarmActivated = false;
volatile bool tempReceived = false;
volatile bool humidReceived = false;
volatile bool gasReceived = false;
volatile bool lightReceived = false;

// Containers to hold values from readings
String temp = "-";
String humid = "-";
String gas = "-";
String light = "-";

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

bool infoReceived() {
  return (tempReceived || humidReceived || gasReceived || lightReceived);
}

void setup() {
  Serial.begin(115200);

  // initialize LCD
  lcd.init();

  // turn on LCD backlight                      
  lcd.backlight();
  lcd.createChar(ARROW_UP, arrow_up_icon);
  lcd.createChar(ARROW_DOWN, arrow_down_icon);
  lcd.createChar(ARROW_UP_PARTIAL, arrow_up_partial_icon);
  lcd.createChar(ARROW_DOWN_PARTIAL, arrow_down_partial_icon);
  lcd.createChar(BELL, bell_icon);
  lcd.createChar(DEGREE, degree_sym);

  printScreen("Initializing,", "Please wait.");

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

  // Wait for ESP32 to connect to MQTT server
  Serial.print("Connecting to MQTT server.");
  while (!mqttClient.isConnected()) {
    Serial.print(".");
    mqttClient.setURI(mqttServer);
    mqttClient.enableLastWillMessage("lwt", "AP going offline");
    mqttClient.setKeepAlive(30);
    mqttClient.loopStart();
    delay(1000);
  }

  printScreen("Init success!");
  delay(1000);
}

void loop() {
  if (infoReceived()) {
    lcd.clear();

    // Temperature
    lcd.setCursor(0, 0);
    lcd.print("Temp: " + temp);
    lcd.write(DEGREE);
    lcd.print("C");

    // Humidity
    lcd.setCursor(0, 1);
    lcd.print("Humidity: " + humid + "%");

    delay(2000);

    lcd.clear();

    // Gas
    lcd.setCursor(0, 0);
    lcd.print("Gas: " + gas);

    // Light
    lcd.setCursor(0, 1);
    lcd.print("Light: " + light);

    delay(2000);

    lcd.clear();
  } else {
    printScreen("Awaiting", "Info...");

    while((!infoReceived()) && (!alarmActivated)) {
      delay(10);
    }
  }

  if (alarmActivated) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(BELL);
    lcd.print(" ATTENTION");
    lcd.setCursor(15, 0);
    lcd.write(BELL);
    lcd.setCursor(0, 1);
    lcd.print("INTRUDER ALERT!");

    // Do not clear the alarm until a signal is received
    while (alarmActivated)
    {
      delay(10);
    }
  
    printScreen("Alarm", "Deactivated");
  }
}

// LCD functions
void printScreen(String str1, String str2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str1);
  lcd.setCursor(0, 1);
  lcd.print(str2);
}

void printScreen(String str) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str);
}

// MQTT functions
void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe(alarmTopic, [](const String &payload)
            {
              if (String(payload) == "1") {
                alarmActivated = true;
              }
            });

        mqttClient.subscribe(clearAlarmTopic, [](const String &payload)
            {
              if (String(payload) == "1") {
                alarmActivated = false;
              }
            });

        mqttClient.subscribe(tempTopic, [](const String &payload)
            {
              tempReceived = true;
              temp = String(payload);
            });

        // Subscribe to humidity topic
        mqttClient.subscribe(humidTopic, [](const String &payload)
          {
              humidReceived = true;
              humid = String(payload); // Assign payload to humid
          });

        // Subscribe to gas concentration topic
        mqttClient.subscribe(gasTopic, [](const String &payload)
          {
              gasReceived = true;
              gas = String(payload); // Assign payload to gas
          });

        // Subscribe to light level topic
        mqttClient.subscribe(lightTopic, [](const String &payload)
          {
              lightReceived = true;
              light = String(payload); // Assign payload to light
          });
    }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}