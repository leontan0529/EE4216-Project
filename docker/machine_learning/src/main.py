from datetime import datetime
import os

# Get the current datetime

# Path to update HausAI's CNN
CNN_OUTPUT_PATH = "./CS3237 Codes/tsh-server/.docker/machine_learning/ml_image/ml_image.txt"
CNN_OUTPUT = "Personnel is not recognised. INTRUDER ALERT!"

# Path to update HausAI's Regression
REGRESSION_OUTPUT_PATH = "./CS3237 Codes/tsh-server/.docker/machine_learning/ml_brightness/ml_brightness.txt"
REGRESSION_OUTPUT = "In 2 hours, the brightness level is expected to be 50 lux. Please turn on the lights."

# HausAI Default Intro
DEFAULT_INTRO = "Welcome back home, Sir.\nI am HausAI, your home assistant. Here is a report for you:\n"

def update_ml(ML_OUTPUT_PATH, ML_OUTPUT):
    current_datetime = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    with open(ML_OUTPUT_PATH, "w") as file:
        file.write(current_datetime)
        file.write("\n")
        file.write(DEFAULT_INTRO)
        file.write(ML_OUTPUT)

# Print current directory
print(os.getcwd())
update_ml(CNN_OUTPUT_PATH, CNN_OUTPUT)
update_ml(REGRESSION_OUTPUT_PATH, REGRESSION_OUTPUT)
