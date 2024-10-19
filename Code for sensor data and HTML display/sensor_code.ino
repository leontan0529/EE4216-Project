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
AsyncWebServer server(80);

const char* ssid = "jonathan";         // Replace with your Wi-Fi SSID
const char* password = "12345678";     // Replace with your Wi-Fi password

// Function declarations
float getDistance();
void readSensorsTask(void *pvParameters);
void handleWebRequest(AsyncWebServerRequest *request);

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


    // Define web server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        handleWebRequest(request);
    });

    server.on("/relay_on", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_PIN, LOW); // Activate relay
        request->send(200, "text/plain", "Relay ON");
    });

    server.on("/relay_off", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_PIN, HIGH); // Deactivate relay
        request->send(200, "text/plain", "Relay OFF");
    });

    server.begin();

    // Create tasks for RTOS
    xTaskCreate(readSensorsTask, "Read Sensors", 2048, NULL, 1, NULL);
}

void loop() {
    // Main loop can be empty as tasks are handled by the web server and RTOS
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

void readSensorsTask(void *pvParameters) {
    for (;;) {
        // Read temperature and humidity
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();

        if (isnan(humidity) || isnan(temperature)) {
            Serial.println("Failed to read from DHT sensor!");
            vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for stability
            continue;
        }

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print(" %\tTemperature: ");
        Serial.print(temperature);
        Serial.println(" *C");

        // Measure distance with HC-SR04
        float distance = getDistance();
        
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");

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

void handleWebRequest(AsyncWebServerRequest *request) {
    String html = "<html><body><h1>Sensor Data</h1>";
    html += "<p>Humidity: " + String(dht.readHumidity()) + " %</p>";
    html += "<p>Temperature: " + String(dht.readTemperature()) + " *C</p>";
    html += "<p>Distance: " + String(getDistance()) + " cm</p>";
    html += "<p><a href=\"/relay_on\">Turn Relay ON</a></p>";
    html += "<p><a href=\"/relay_off\">Turn Relay OFF</a></p>";
    html += "</body></html>";
    
    request->send(200, "text/html", html);
}