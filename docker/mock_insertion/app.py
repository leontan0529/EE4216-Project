import random
from datetime import datetime, timedelta
import time
import os

# Initialize sensor values
temperature = round(random.uniform(18.0, 26.0), 2)
humidity = round(random.uniform(30.0, 60.0), 2)
light_intensity = round(random.uniform(0.0, 800.0), 2)
gas_concentration = round(random.uniform(0.0, 15.0), 2)

# Directory Structure
DATA_DIR = "/prometheus_data"

def generate_fake_data():
    global temperature, humidity, light_intensity, gas_concentration  # Declare sensor variables as global
    current_time = datetime.now()
    
    # Simulate DHT22 data (Temperature and Humidity)
    if random.choice([True, False]):
        temperature += round(random.uniform(0.1, 1.0), 2)   # Gradually increase temperature
        humidity += round(random.uniform(0.1, 1.0), 2)      # Gradually increase humidity
    else:
        temperature -= round(random.uniform(0.1, 1.0), 2)   # Gradually decrease temperature
        humidity -= round(random.uniform(0.1, 1.0), 2)      # Gradually decrease humidity
   
    # Allow for fluctuations even after reaching bounds
    temperature += round(random.uniform(-0.5, 0.5), 2)   # Small fluctuations around current value
    humidity += round(random.uniform(-0.5, 0.5), 2)      # Small fluctuations around current value
    
    # Ensure values stay within realistic ranges
    temperature = max(min(temperature, 26.0), 18.0)
    humidity = max(min(humidity, 60.0), 30.0)
    
    # Simulate Light Sensor data (Light intensity)
    hour = current_time.hour
    if hour >= 6 and hour < 18: 
        light_intensity += round(random.uniform(-20.0, 20.0), 2) # More significant fluctuations during daytime
    else: 
        light_intensity += round(random.uniform(-10.0, 10.0), 2)   # Smaller fluctuations during nighttime
    
    light_intensity = max(min(light_intensity, 800.0), 0.0)   # Keep within bounds
    
    # Simulate MQ2 Sensor data (Gas concentration)
    gas_concentration += round(random.gauss(0, 1), 2)   # Small fluctuations in gas concentration
    gas_concentration = max(min(gas_concentration, 60.0), 0.0)

    # Save Data to tem.log
    tem_dir = os.path.join(DATA_DIR, current_time.strftime("%Y-%m-%d/%H"), "tem")
    os.makedirs(tem_dir, exist_ok=True)
    tem_file = os.path.join(tem_dir, "tem.log")
    with open(tem_file, "a") as f:
        f.write(f"{current_time.strftime('%Y-%m-%d %H:%M:%S')} {temperature:.2f}\n")

    # Save Data to hum.log
    hum_dir = os.path.join(DATA_DIR, current_time.strftime("%Y-%m-%d/%H"), "hum")
    os.makedirs(hum_dir, exist_ok=True)
    hum_file = os.path.join(hum_dir, "hum.log")
    with open(hum_file, "a") as f:
        f.write(f"{current_time.strftime('%Y-%m-%d %H:%M:%S')} {humidity:.2f}\n")

    # Save Data to lux.log
    lux_dir = os.path.join(DATA_DIR, current_time.strftime("%Y-%m-%d/%H"), "lux")
    os.makedirs(lux_dir, exist_ok=True)
    lux_file = os.path.join(lux_dir, "lux.log")
    with open(lux_file, "a") as f:
        f.write(f"{current_time.strftime('%Y-%m-%d %H:%M:%S')} {light_intensity:.2f}\n")

    # Save Data to gas.log
    gas_dir = os.path.join(DATA_DIR, current_time.strftime("%Y-%m-%d/%H"), "gas")
    os.makedirs(gas_dir, exist_ok=True)
    gas_file = os.path.join(gas_dir, "gas.log")
    with open(gas_file, "a") as f:
        f.write(f"{current_time.strftime('%Y-%m-%d %H:%M:%S')} {gas_concentration:.2f}\n")
    
    # Format the output for all sensors with two decimal places
    print(f"{current_time.strftime('%Y-%m-%d %H:%M:%S')} "
          f"{temperature:.2f} {humidity:.2f} {light_intensity:.2f} {gas_concentration:.2f}")
    
    time.sleep(60)   # Pause for one second before the next iteration

if __name__ == "__main__":
    generate_fake_data()