# Use the official Alpine image as the base image
FROM alpine:latest

# Install Mosquitto client and curl
RUN apk add --no-cache mosquitto-clients curl

# Set the working directory in the container
WORKDIR /app

# Copy the shell script into the container
COPY . /app

# Make the shell script executable
RUN chmod +x ./curl.sh

# Set the entrypoint to the shell script
ENTRYPOINT ["./curl.sh"]
