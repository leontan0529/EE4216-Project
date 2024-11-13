#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include <LiquidCrystal_I2C.h>
#include "custom_char.h"
#include "esp_timer.h"

#define PIR_SENSOR_PIN 18

// // Network credentials
// const char* ssid = "ESP32-AP";
// const char* password = "12345678";

const char* ssid = "khai_rizz";
const char* password = "handsumboi";

// Static IP configuration
IPAddress local_IP(192, 168, 157, 120);
IPAddress gateway(192, 168, 157, 138);
IPAddress subnet(255, 255, 255, 0);

// MQTT details
const char* mqttServer = "mqtt://192.168.157.167:1883";

char *alarmTopic = "img";
char *clearAlarmTopic = "deactivate";
char *tempTopic = "tem";       // Topic for temperature
char *humidTopic = "hum"; // Topic for humidity
char *gasTopic = "gas";         // Topic for gas concentration
char *lightTopic = "lux";     // Topic for light level
char *pirTopic = "pir";

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

const int64_t TIME_PIR = 10000000;
int64_t lastTimePir = esp_timer_get_time();

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

const int64_t STD_SCREEN_TIME = 2000000;
int64_t currentTime = esp_timer_get_time();

enum SCREEN_ID {
  AWAIT_INFO_SCREEN,
  DISPLAY_INFO_SCREEN_1,
  DISPLAY_INFO_SCREEN_2,
  ALARM_SCREEN,
  DEACTIVATE_SCREEN
};

SCREEN_ID currentScreen = AWAIT_INFO_SCREEN;

String prev_str1, prev_str2;

bool isNextScreen() {
  if ((esp_timer_get_time() - currentTime) >= STD_SCREEN_TIME) {
    currentTime = esp_timer_get_time();
    return true;
  } else {
    return false;
  }
}

void changeScreen(SCREEN_ID newScreen) {
  currentScreen = newScreen;
  currentTime = esp_timer_get_time(); 
  lcd.clear();
}

bool infoReceived() {
  return (tempReceived || humidReceived || gasReceived || lightReceived);
}

/*
  DARK = 3500
  BRIGHT = 1000
*/

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
  // WiFi.mode(WIFI_AP);

  // // Configure the static IP address
  // if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
  //   Serial.println("Failed to configure static IP address");
  // }
  
  // // Configure the access point
  // WiFi.softAP(ssid, password);

  // // Print the IP address of the access point
  // Serial.print("Access Point IP address: ");
  // Serial.println(WiFi.softAPIP());

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to configure static IP address");
  }

  // Presence Detection
  pinMode(PIR_SENSOR_PIN, INPUT);

  /*----- Establish WiFi connection -----*/
  WiFi.begin(ssid, password);
  WiFi.setHostname("ESP32Main");

  // Wait for ESP32 to connect to WiFi before connecting to MQTT client
  Serial.print("Connecting.");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  
  Serial.println("\nConnected to WiFi!");
  /*----------*/
  // Wait for ESP32 to connect to MQTT server
  Serial.print("Connecting to MQTT server.");
  while (!mqttClient.isConnected()) {
    Serial.print(".");
    mqttClient.setURI(mqttServer);
    mqttClient.enableLastWillMessage("lwt", "AP going offline");
    mqttClient.setKeepAlive(60);
    mqttClient.loopStart();
    delay(1000);
  }

  printScreen("Init success!");
  delay(1000);
}

void loop() {
  //Presence Detection
  if ((esp_timer_get_time() - lastTimePir) >= TIME_PIR) {
    int sensorValue = digitalRead(PIR_SENSOR_PIN);
    if (sensorValue == HIGH) {
      Serial.println("Motion detected!");
      mqttClient.publish(pirTopic, "1", 0, false);
    } else {
      Serial.println("No motion detected.");
      mqttClient.publish(pirTopic, "0", 0, false);
    }

    lastTimePir = esp_timer_get_time();
  }

  if (currentScreen == AWAIT_INFO_SCREEN) {
    printScreen("Awaiting", "Info...");

    if (infoReceived()) {
      changeScreen(DISPLAY_INFO_SCREEN_1);
    }
  }

  if (currentScreen == DISPLAY_INFO_SCREEN_1) {
    // Screen 1

    // Temperature
    lcd.setCursor(0, 0);
    lcd.print("Temp: " + temp);
    lcd.write(DEGREE);
    lcd.print("C");

    // Humidity
    lcd.setCursor(0, 1);
    lcd.print("Humidity: " + humid + "%");

    if (isNextScreen()) {
      changeScreen(DISPLAY_INFO_SCREEN_2);
    }
  }

  if (currentScreen == DISPLAY_INFO_SCREEN_2) {
    String light_level;

    if (light.toInt() >= 3500) {
      light_level = "BRIGHT";
    } else if (light.toInt() <= 1000) {
      light_level = "DARK";
    } else {
      light_level = "AMBIENT";
    }
    printScreen("Gas: " + gas + "ppm", "Light: " + light_level);

    if (isNextScreen()) {
      changeScreen(DISPLAY_INFO_SCREEN_1);
    }
  }

  if (alarmActivated) {
    lcd.setCursor(0, 0);
    lcd.write(BELL);
    lcd.print(" ATTENTION");
    lcd.setCursor(15, 0);
    lcd.write(BELL);
    lcd.setCursor(0, 1);
    lcd.print("INTRUDER ALERT!");
  }

  if (currentScreen == DEACTIVATE_SCREEN) {
    printScreen("Alarm", "Deactivated");

    if (isNextScreen()) {
      changeScreen(DISPLAY_INFO_SCREEN_1);
    }
  }
}

// LCD functions
void printScreen(String str1, String str2) {
  if ((str1 != prev_str1) && (str2 != prev_str2)) {
    lcd.clear();
  }
  
  lcd.setCursor(0, 0);
  lcd.print(str1);
  lcd.setCursor(0, 1);
  lcd.print(str2);

  prev_str1 = str1;
  prev_str2 = str2;
}

void printScreen(String str) {
  if (str != prev_str1) {
    lcd.clear();
  }

  lcd.setCursor(0, 0);
  lcd.print(str);

  prev_str1 = str;
}

// MQTT functions
void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe(alarmTopic, [](const String &payload)
            {
              if (String(payload) == "2") {
                alarmActivated = true;
                changeScreen(ALARM_SCREEN);
              } else {
                alarmActivated = false;
                changeScreen(DEACTIVATE_SCREEN);
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