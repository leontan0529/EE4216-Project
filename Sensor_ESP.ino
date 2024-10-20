#include <WiFi.h>
#include <ESPAsyncWebSrv.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>

#define DHTPIN 4           // DHT22 data pin
#define DHTTYPE DHT22      // DHT type
#define TRIG_PIN 5        // HC-SR04 trigger pin
#define ECHO_PIN 18       // HC-SR04 echo pin
#define RELAY_PIN 16      // Relay control pin
#define RGB_PIN 17        // RGB LED pin
#define NUM_PIXELS 1      // Number of pixels in RGB LED stick

DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "ESP32_AP";         // Access Point SSID
const char* password = "12345678";     // Access Point Password
const char* accessPointHost = "http://192.168.4.1"; // Replace with your Access Point IP address

// Function declarations
float getDistance();
void sendDataToAccessPoint(float humidity, float temperature);
void readSensorsTask(void *pvParameters);

void setup() {
    Serial.begin(115200);
    dht.begin();
    pixels.begin();

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(RELAY_PIN, OUTPUT);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Connected to WiFi");

    // Create tasks for RTOS
    xTaskCreate(readSensorsTask, "Read Sensors", 2048, NULL, 1, NULL);
}

void loop() {
// Main loop can be empty as tasks are handled by the web server and RTOS
}

void readSensorsTask(void *pvParameters) {
    for (;;) {
        // Read temperature and humidity
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();
        float distance = getDistance();

        if (!isnan(humidity) && !isnan(temperature)) {
            sendDataToAccessPoint(humidity, temperature, distance);
            Serial.print("Humidity: ");
            Serial.print(humidity);
            Serial.print(" %\tTemperature: ");
            Serial.print(temperature);
            Serial.print(" *C\tDistance: ");
            Serial.print(distance);
            Serial.println(" cm");
        } else {
            Serial.println("Failed to read from DHT sensor!");
        }
        delay(10000); // Send data every 10 seconds
    }

        // Control relay based on distance or other conditions
        if (distance < 10) { // Assuming <10 cm indicates an intruder
            digitalWrite(RELAY_PIN, LOW); // Activate relay (unlock)
            pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red color for intruder alert
            pixels.show();
            Serial.println("Intruder detected! Relay activated.");
            vTaskDelay(5000 / portTICK_PERIOD_MS); // Keep relay active for a while
            digitalWrite(RELAY_PIN, HIGH); // Deactivate relay (lock)
            pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Turn off RGB LED
            pixels.show();
        } else {
            digitalWrite(RELAY_PIN, HIGH); // Ensure relay is off when no intruder
            pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green color when safe
            pixels.show();
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay before next reading
    }
}

float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    return (duration * 0.0343) / 2; // Calculate distance in cm
}

void sendDataToAccessPoint(float humidity, float temperature, float distance) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(accessPointHost) + "/sensor_data";

        String payload = "humidity=" + String(humidity) + "&temperature=" + String(temperature) + "&distance=" + String(distance);

        http.begin(url); 
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        int httpResponseCode = http.POST(payload); 

        if (httpResponseCode > 0) {
            String response = http.getString(); 
            Serial.println(httpResponseCode);   
            Serial.println(response);            
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }
        
        http.end(); 
    }
}