#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_camera.h>

// Camera configuration
camera_config_t config = {
    .pin_pwdn = -1,
    .pin_reset = -1,
    .pin_xclk = 21,
    .pin_sccb_sda = 26,
    .pin_sccb_scl = 27,
    .pin_d7 = 35,
    .pin_d6 = 34,
    .pin_d5 = 39,
    .pin_d4 = 36,
    .pin_d3 = 19,
    .pin_d2 = 18,
    .pin_d1 = 5,
    .pin_d0 = 17,
    .pin_vsync = 25,
    .pin_href = 23,
    .pin_pclk = 22,

    .xclk_freq_hz = 20000000,
    .ledc_channel = 0,
    .ledc_timer = 0,
    .pixel_format = PIXFORMAT_JPEG, // Use JPEG format
    .frame_size = FRAMESIZE_SVGA,   // Use SVGA resolution (800x600)
    .jpeg_quality = 12,              // JPEG quality (lower is better)
    .fb_count = 2                    // Use two frame buffers
};

const char* ssid = "ESP32_AP";         // Access Point SSID
const char* password = "12345678";     // Access Point Password
const char* accessPointHost = "http://192.168.4.1"; // Replace with your Access Point IP address

void setup() {
    Serial.begin(115200);
    
    // Initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    
    Serial.println("Connected to WiFi");
}

void loop() {
    camera_fb_t *fb = esp_camera_fb_get();
    
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    sendImageToAccessPoint(fb->buf, fb->len);

    esp_camera_fb_return(fb); // Return frame buffer back to driver

    delay(10000); // Capture image every 10 seconds
}

void sendImageToAccessPoint(uint8_t *imageData, size_t imageSize) {
   if (WiFi.status() == WL_CONNECTED) {
       HTTPClient http;
       String url = String(accessPointHost) + "/upload"; // Ensure this endpoint exists on your AP ESP32

       http.begin(url); 
       http.addHeader("Content-Type", "application/octet-stream");

       int httpResponseCode = http.POST(imageData, imageSize); 

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