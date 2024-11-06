# Use the official Eclipse Mosquitto image as the base
FROM eclipse-mosquitto:latest

# Copy the Mosquitto configuration file
COPY ./config/mosquitto.conf /mosquitto/config/mosquitto.conf

# Copy the subscription script
COPY ./subscribe.sh /mosquitto/subscribe.sh

# Run the subscription script when the container starts
CMD ["sh", "-c", "/mosquitto/subscribe.sh & mosquitto -c /mosquitto/config/mosquitto.conf"]
