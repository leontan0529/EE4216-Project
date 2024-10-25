#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebSrv.h>

const char* ssid = "ESP32_AP";         // Access Point SSID
const char* password = "12345678";     // Access Point Password
const char* rpiHost = "http://<Raspberry_Pi_IP>:5000"; // Replace with your Raspberry Pi IP address

AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);
    
    // Set up the access point
    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP()); // Print the IP address of the AP

    // Define web server routes
    server.on("/sensor_data", HTTP_POST, [](AsyncWebServerRequest *request){
        String humidity = request->arg("humidity");
        String temperature = request->arg("temperature");
        Serial.print("Received Sensor Data - Humidity: ");
        Serial.print(humidity);
        Serial.print(", Temperature: ");
        Serial.println(temperature);
        
        // Send data to Raspberry Pi
        sendDataToRaspberryPi(humidity, temperature);
        
        request->send(200, "text/plain", "Sensor data received");
    });

    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Image uploaded");
    }, handleFileUpload); // Handle file upload

    server.begin();
}

void loop() {
    // Main loop can be empty
}

void handleFileUpload(AsyncWebServerRequest *request) {
    if (request->hasParam("image", true)) {
        AsyncWebParameter* p = request->getParam("image", true);
        
        // Here you can save the image data or process it as needed.
        Serial.println("Image uploaded!");
        
        // Send image to Raspberry Pi or handle it as needed
        sendImageToRaspberryPi(p->value().c_str(), p->size());
        
        request->send(200, "text/plain", "Image uploaded successfully");
        
    } else {
        request->send(400, "text/plain", "No image uploaded");
    }
}

void sendDataToRaspberryPi(String humidity, String temperature) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(rpiHost) + "/sensor_data";

        String payload = "humidity=" + humidity + "&temperature=" + temperature;

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

void sendImageToRaspberryPi(const char* imageData, size_t imageSize) {
   if (WiFi.status() == WL_CONNECTED) {
       HTTPClient http;
       String url = String(rpiHost) + "/upload"; // Ensure this endpoint exists on your Raspberry Pi

       http.begin(url); 
       http.addHeader("Content-Type", "application/octet-stream");

       int httpResponseCode = http.POST((uint8_t*)imageData, imageSize); 

       if (httpResponseCode > 0) {
           String response = http.getString(); 
           Serial.println(httpResponseCode);   
           Serial.println(response);            
       } else {
           Serial.print("Error on sending image POST: ");
           Serial.println(httpResponseCode);
       }
       
       http.end(); 
   }
}