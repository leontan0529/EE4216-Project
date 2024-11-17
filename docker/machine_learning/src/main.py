import os
import pandas as pd
from models.random_forest import train_random_forest
from models.linear_regression import train_linear_regression
from datetime import datetime, timezone
import sys
import re
import requests
from apscheduler.schedulers.background import BackgroundScheduler
from models.cnn import train_cnn
from recognition import recognize_face
import threading
import time
import joblib
from models.cnn import load_model
from data.preprocess import load_log_file
from data.preprocess import load_data_from_folders

# Add the path to the project directory for easier imports
sys.path.append('/Users/constanceow/Desktop/EE4216-Project/docker/machine_learning/src/src')
import constants

def process_and_recognize(model):
    """
    Periodically checks the image folder for new images to process and recognize faces.
    
    Args:
        model: The pre-trained model used for face recognition.
    """
    image_folder = '/Users/constanceow/Desktop/EE4216-Project/docker/prometheus_data/images'
    processed_images = set()  # Track processed images to avoid reprocessing
    
    while True:
        # List all files in the image folder
        for filename in os.listdir(image_folder):
            if filename.endswith(('.png', '.jpg', '.jpeg')):  # Check if the file is an image
                image_path = os.path.join(image_folder, filename)

                # Only process the image if it hasn't been processed before
                if image_path not in processed_images:
                    print(f"Processing image: {image_path}")
                    
                    # Call face recognition function
                    check_and_recognize_faces(model, image_path)  
                    
                    # Mark this image as processed
                    processed_images.add(image_path)

        time.sleep(5)  # Check for new images every 5 seconds


def train_models():
    """
    Trains the Random Forest and Linear Regression models on lux data and saves them.
    """
    best_rf_model_path = constants.BEST_RF_MODEL_PATH
    best_lr_model_path = constants.BEST_LR_MODEL_PATH

    # Preprocess lux_df to extract hour and value for training
    lux_df['timestamp'] = pd.to_datetime(lux_df['timestamp'])  # Ensure 'timestamp' is in datetime format
    lux_df['hour'] = lux_df['timestamp'].dt.hour  # Extract the hour from the timestamp
    X = lux_df[['hour']]  # Features: hour of day
    y = lux_df['value']  # Target: lux values
    
    # Generate a name for the model based on the current timestamp
    pattern = r'_(\d{8}_\d{6})\.pkl$'
    base_model_name = re.sub(pattern, '', os.path.basename(best_rf_model_path))

    model_directory = constants.MODEL_PATH
    os.makedirs(model_directory, exist_ok=True)  # Create directory if it doesn't exist

    new_timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")  # Generate timestamp for model name

    model_save_path_rfc = os.path.join(model_directory, f'{base_model_name}_{new_timestamp}.pkl')
    model_save_path_lr = os.path.join(model_directory.replace('RF', 'LR'), f'trained_LR_model_{new_timestamp}.pkl')

    # Train and save the Random Forest Classifier
    train_random_forest(X, y, model_save_path_rfc)

    # Train and save the Linear Regression Model
    train_linear_regression(X, y, model_save_path_lr)

    return lux_df  # Return the processed lux DataFrame for further use


def check_and_recognize_faces(image_path, model):
    """
    Recognizes faces in the given image using the specified model.
    
    Args:
        image_path (str): The path to the image.
        model: The pre-trained CNN model used for face recognition.
        
    Returns:
        str: The output of the face recognition (who is in the image).
    """
    CNN_OUTPUT = ""

    # Check if the image exists
    if os.path.exists(image_path):
        print(f"Processing image: {image_path}")

        # Known faces dictionary (0: User A, 1: User B, etc.)
        known_faces = {
            0: "User A",
            1: "User B",
            2: "User C",
        }
        
        # Call the face recognition function and capture the result
        CNN_OUTPUT = recognize_face(image_path, known_faces, model)
        
    else:
        print(f"No image found at {image_path} for recognition.")
    
    return CNN_OUTPUT


def update_ml(ML_OUTPUT_PATH, ML_OUTPUT):
    """
    Updates the ML output file with the current datetime and ML output.
    
    Args:
        ML_OUTPUT_PATH (str): Path to the file where the output should be written.
        ML_OUTPUT (str): The output (e.g., face recognition result or recommendation).
    """
    current_datetime = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    # Write to the output file
    with open(ML_OUTPUT_PATH, "w") as file:
        file.write(current_datetime)
        file.write("\n")
        file.write(DEFAULT_INTRO)
        file.write(ML_OUTPUT)


# Function to check if there are new .jpg files in the images directory
def new_jpg(images_dir):
    """
    Get the name of the most recently modified JPEG file in the directory.
    
    Args:
        images_dir (str): The directory to check for new .jpg files.
        
    Returns:
        str: The filename of the most recently modified JPEG, or None if no JPEG is found.
    """
    jpg_files = [f for f in os.listdir(images_dir) if f.endswith(('.jpg', '.jpeg'))]
    
    if not jpg_files:
        return None  # Return None if there are no JPEG files

    # Get the full paths and modification times
    jpg_paths = [os.path.join(images_dir, f) for f in jpg_files]
    most_recent_file = max(jpg_paths, key=os.path.getmtime)  # Get the most recent file by modification time
    
    return os.path.basename(most_recent_file)  # Return just the filename


def has_lux_entries_changed(lux_df, previous_lux_df):
    """
    Compare the current lux DataFrame with the previous one to detect changes.
    
    Args:
        lux_df (DataFrame): The current lux DataFrame.
        previous_lux_df (DataFrame): The previous lux DataFrame.
        
    Returns:
        bool: True if the lux DataFrame has changed, False otherwise.
    """
    # Check if the shapes of the DataFrames are different
    if lux_df.shape != previous_lux_df.shape:
        return True
    
    # Check if any values in the DataFrames are different
    if not lux_df.equals(previous_lux_df):
        return True
    
    return False

if __name__ == '__main__':
    images_dir = '/Users/constanceow/Desktop/EE4216-Project/docker/prometheus_data/images'
    
    last_image_name = None  # Initialize to None or an empty string
    
    previous_lux_df = pd.DataFrame()  # Initialize an empty DataFrame for previous lux data

    CNN_OUTPUT_PATH = "..//ml_image/ml_image.txt"
    
    DEFAULT_INTRO = "Welcome back home, Sir.\nI am HausAI, your home assistant. Here is a report for you:\n"

    # Main loop to periodically check for new images and process lux data
    while True:
        
        # Check if the most recent JPEG has changed
        current_image_name = new_jpg(images_dir)
        if current_image_name != last_image_name:
            print(f"Most recent JPEG has changed: {current_image_name}")
            last_image_name = current_image_name
            
            cnn_data_dir = '/Users/constanceow/Desktop/EE4216-Project/docker/machine_learning'
            resume_training = True
            
            # Load the trained model (optionally train if needed)
            model = load_model()  
            
            # Perform face recognition on the new image
            CNN_OUTPUT = check_and_recognize_faces(os.path.join(images_dir, current_image_name), model)
            update_ml(CNN_OUTPUT_PATH, CNN_OUTPUT)

        # Load data from folders for lux_df (temperature, humidity, gas, etc.)
        base_folder = "/Users/constanceow/Desktop/EE4216-Project/docker/prometheus_data"
        gas_df, hum_df, lux_df, tem_df = load_data_from_folders(base_folder)

        # If there are new lux entries, make predictions using linear regression
        if not previous_lux_df.empty and has_lux_entries_changed(lux_df, previous_lux_df):
            print("New entries detected in lux DataFrame.")
            
            # Load the trained Linear Regression model
            model_lr = joblib.load(constants.BEST_LR_MODEL_PATH)
            
            lux_df['timestamp'] = pd.to_datetime(lux_df['timestamp'])  # Convert timestamps to datetime
            lux_df['hour'] = lux_df['timestamp'].dt.hour  # Extract hour from timestamp
            
            new_data = lux_df[['hour', 'value']]  # Use hour and lux value for prediction
            
            # Make predictions using the linear regression model
            y_pred = model_lr.predict(new_data)
            
            REGRESSION_OUTPUT = ""
            
            # Provide recommendation based on predicted lux value
            if 3000 > y_pred[0] > 300: 
                REGRESSION_OUTPUT = "Recommend to turn off lights."
            elif y_pred[0] > 3000: 
                REGRESSION_OUTPUT = "In 2 hours, the brightness level is expected to be 50 lux. Please turn on the lights."
            elif y_pred[0] < 300:
                REGRESSION_OUTPUT = "No recommendation."

            # Update the regression output file
            REGRESSION_OUTPUT_PATH = "..//ml_brightness/ml_brightness.txt"
            update_ml(REGRESSION_OUTPUT_PATH, REGRESSION_OUTPUT)
        
        # Store the current lux_df as previous for next iteration
        previous_lux_df = lux_df.copy()
        time.sleep(10)  # Wait before checking again (adjust as needed)


# Set up a scheduler to train models periodically (every hour)
scheduler = BackgroundScheduler()
scheduler.add_job(train_models, 'cron', minute=0)
scheduler.start()

print("Scheduler started. Waiting for scheduled jobs and image recognition...")

try:
    while True:
        time.sleep(1)  # Sleep to reduce CPU usage

except (KeyboardInterrupt, SystemExit):
    print("Shutting down...")
    scheduler.shutdown()
