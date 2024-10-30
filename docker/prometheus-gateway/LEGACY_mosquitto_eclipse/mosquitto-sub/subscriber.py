import os
import paho.mqtt.client as mqtt
import time

broker_address = "mqtt"
topics = ["esp32/tem", "esp32/hum", "esp32/gas", "esp32/brightness"]
date = time.strftime("%Y-%m-%d")
hour = time.strftime("%H")
date_time = time.strftime("%Y-%m-%d %H:%M:%S")

# directory structure for datapoints: dir_path: date/hour/topic.txt 

def on_message(client, userdata, message):
    topic = message.topic.split('/')[-1]
    payload = message.payload.decode('utf-8')
    print(f"Received `{payload}` from `{topic}` topic")

    # Check if directory exists, if not create it
    directory = f"../../prometheus_data/{date}/{hour}"
    if not os.path.exists(directory):
        os.makedirs(directory)

    # Save data to respective topic
    with open(f"{directory}/{topic}.txt", "a") as file:
        file.write(f"{payload}\n")

client = mqtt.Client("SubscriberClient")
client.on_message = on_message
client.connect(broker_address)

# Subscribe to topics
for topic in topics:
    client.subscribe(topic)

client.loop_forever()

