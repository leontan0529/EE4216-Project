#!/bin/bash

# IP address/endpoint to curl
ENDPOINT="http://192.168.4.5/saved-photo"

# Function to handle incoming MQTT messages
handle_message() {
    echo "Message received: $1"
    # Create the directory structure based on the current date and time
    now=$(date +"%Y-%m-%d/%H")
    directory_path="/prometheus_data/$now"
    mkdir -p "$directory_path"
    
    # Create the file path
    file_path="$directory_path/$(date +"%Y%m%d_%H%M%S").jpg"
    
    # Perform the curl request and save the image
    curl -o "$file_path" $ENDPOINT
    echo "Image successfully downloaded and saved as '$file_path'"
}

# Subscribe to the MQTT topic and handle messages
mosquitto_sub -h mqtt_broker_address -t mqtt_topic | while read -r message; do
    handle_message "$message"
done
