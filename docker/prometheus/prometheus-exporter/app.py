import time
import os
from prometheus_client import start_http_server, Gauge
from datetime import datetime, timedelta
import logging
import sys

# Configure logging to output to stdout for Docker compatibility
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)  # Log level set to DEBUG to capture all logs

# Create a stream handler to send log output to stdout
stdout_handler = logging.StreamHandler(sys.stdout)
stdout_handler.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
stdout_handler.setFormatter(formatter)
logger.addHandler(stdout_handler)

# Define Prometheus metric types
TEMPERATURE = Gauge('temperature', 'Temperature of the sensor', ['sensor'])
LUX = Gauge('lux', 'Lux value of the sensor', ['sensor'])
HUMIDITY = Gauge('humidity', 'Humidity of the sensor', ['sensor'])
GAS = Gauge('gas', 'Gas levels of the sensor', ['sensor'])
PIR = Gauge('motion', 'PIR sensor status', ['sensor'])
IMG = Gauge('intrusion', 'Door intrusion status', ['sensor'])

# Track the latest timestamp for each sensor to manage staleness
last_update = {
    "tem": None,
    "lux": None,
    "hum": None,
    "gas": None,
    "pir": None,
    "img": None
}

# Threshold for considering data stale (e.g., 1 minute)
STALE_THRESHOLD = timedelta(minutes=1)

# Define the function to read logs and expose metrics
def read_logs_and_expose_metrics():
    data_dir = "/prometheus_data"
    sensor_types = ["tem", "lux", "hum", "gas", "pir", "img"]
    new_data_found = False  # Flag to detect if any new data is found

    for sensor in sensor_types:
        # Define the directory for each sensor type, based on the current date and hour
        sensor_dir = os.path.join(data_dir, datetime.now().strftime("%Y-%m-%d/%H"), sensor)
        
        # Check if the sensor directory exists
        if not os.path.exists(sensor_dir):
            logger.warning(f"Sensor directory {sensor_dir} does not exist.")
            continue

        logger.info(f"Reading data from {sensor_dir}")
        
        # Process each file in the sensor directory
        for log_file in os.listdir(sensor_dir):
            log_path = os.path.join(sensor_dir, log_file)
            
            if not os.path.isfile(log_path):
                logger.warning(f"Log file {log_path} is not a valid file.")
                continue

            with open(log_path, 'r') as f:
                for line in f:
                    line = line.strip()  # Remove any extra whitespace or line breaks

                    # Skip empty or malformed lines
                    if not line:
                        logger.warning("Skipping empty line.")
                        continue

                    try:
                        # Expecting format: YYYY-MM-DD HH:MM:SS value
                        timestamp_str, value_str = line.rsplit(" ", 1)
                        timestamp = datetime.strptime(timestamp_str, "%Y-%m-%d %H:%M:%S")
                        value = float(value_str)  # Convert value to float

                        # Log details about each metric processed
                        logger.debug(f"Processing {sensor} log: {timestamp} -> {value}")

                        # Update the metric and last update time only if this timestamp is newer
                        if last_update[sensor] is None or timestamp > last_update[sensor]:
                            last_update[sensor] = timestamp
                            new_data_found = True  # Mark that we have new data

                            # Update Prometheus metrics without timestamp (as Prometheus does not support backfilling via scraping)
                            if sensor == "tem":
                                TEMPERATURE.labels(sensor=sensor).set(value)
                            elif sensor == "lux":
                                LUX.labels(sensor=sensor).set(value)
                            elif sensor == "hum":
                                HUMIDITY.labels(sensor=sensor).set(value)
                            elif sensor == "gas":
                                GAS.labels(sensor=sensor).set(value)
                            elif sensor == "pir":
                                PIR.labels(sensor=sensor).set(value)
                            elif sensor == "img":
                                IMG.labels(sensor=sensor).set(value)

                    except ValueError as e:
                        logger.error(f"Error parsing line: {line}. Exception: {str(e)}")

    return new_data_found

# Function to reset metrics if they are stale
def reset_stale_metrics():
    current_time = datetime.now()
    for sensor, last_time in last_update.items():
        if last_time is None or current_time - last_time > STALE_THRESHOLD:
            # Reset the metric to indicate no recent data
            logger.info(f"Resetting stale metric for sensor: {sensor}")
            if sensor == "tem":
                TEMPERATURE.labels(sensor=sensor).set(float('nan'))
            elif sensor == "lux":
                LUX.labels(sensor=sensor).set(float('nan'))
            elif sensor == "hum":
                HUMIDITY.labels(sensor=sensor).set(float('nan'))
            elif sensor == "gas":
                GAS.labels(sensor=sensor).set(float('nan'))
            elif sensor == "pir":
                PIR.labels(sensor=sensor).set(float('nan'))
            elif sensor == "img":
                IMG.labels(sensor=sensor).set(float('nan'))

# Start Prometheus HTTP server
def start_exporter():
    # Expose metrics on port 9092 (you can change the port if needed)
    start_http_server(9092)
    logger.info("Prometheus Exporter started on port 9092.")
    
    # First read and expose historical data
    logger.info("Reading and exposing historical data for backfilling.")
    read_logs_and_expose_metrics()

    # Now, continuously read new data and expose it
    while True:
        new_data_found = read_logs_and_expose_metrics()
        if not new_data_found:
            reset_stale_metrics()  # Only reset if no new data was found
        time.sleep(30)  # Check every 15 seconds for new data and stale metrics

if __name__ == "__main__":
    start_exporter()