import os
import time
import requests
from datetime import datetime
import paho.mqtt.client as mqtt

# Environment variables
mqtt_broker = os.getenv("MQTT_BROKER", "localhost")
mqtt_topic = os.getenv("MQTT_TOPIC", "img")
esp32_server_url = os.getenv("ESP32_SERVER", "http://192.168.4.5/saved-photo")  # Replace with actual ESP32 IP

# Callback when connecting to MQTT broker
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code", rc)
    # Subscribe to the specified topic
    client.subscribe(mqtt_topic)

# Callback when a message is received
def on_message(client, userdata, msg):
    print("Message received on topic:", msg.topic)
    print("Message:", msg.payload.decode())

    # Perform HTTP GET request to ESP32 server
    try:
        response = requests.get(esp32_server_url) 
        if response.status_code == 200:
            # Get current date and time for directory structure and filename
            now = datetime.now()
            date_str = now.strftime("%Y-%m-%d")
            hour_str = now.strftime("%H")
            timestamp_str = now.strftime("%Y%m%d_%H%M%S")

            # Directory path and filename
            save_dir = f"/prometheus_data/{date_str}/{hour_str}"
            os.makedirs(save_dir, exist_ok=True)  # Create directories if they don't exist
            file_path = os.path.join(save_dir, f"{timestamp_str}.jpeg")

            # Save the image
            with open(file_path, "wb") as f:
                f.write(response.content)
            print(f"Image saved to {file_path}")

        else:
            print("Failed to retrieve image. Status code:", response.status_code)
    except requests.RequestException as e:
        print("Error in HTTP GET request:", e)

# Set up the MQTT client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
client.connect(mqtt_broker, 1883, 60)

# Blocking loop to process network traffic, dispatch callbacks, and handle reconnections
client.loop_start()

# Keep the container running
while True:
    time.sleep(10)