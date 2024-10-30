#!/bin/sh

# Wait for Mosquitto to start
sleep 5

# Function to create directories and save data
save_data() {
  local topic=$1
  local payload=$2
  local timestamp=$(date +'%Y-%m-%d %H:%M:%S')
  local date=$(date +'%Y-%m-%d')
  local hour=$(date +'%H')
  local dir="../../prometheus_data/$date/$hour/$topic"

  mkdir -p "$dir"
  echo "$timestamp $payload" >> "$dir/$topic.log"
}

# Subscribe to topics and save data with timestamps
mosquitto_sub -h localhost -t "tem" | while read -r payload; do
  save_data "tem" "$payload"
done &

mosquitto_sub -h localhost -t "hum" | while read -r payload; do
  save_data "hum" "$payload"
done &

mosquitto_sub -h localhost -t "gas" | while read -r payload; do
  save_data "gas" "$payload"
done &

mosquitto_sub -h localhost -t "lux" | while read -r payload; do
  save_data "lux" "$payload"
done &
