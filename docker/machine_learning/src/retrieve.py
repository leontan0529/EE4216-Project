import os
import pandas as pd

CACHE_FILE_PATH = 'data_cache.txt'  # Path for your cache file

def get_cached_length():
    """Retrieve the cached length from the file."""
    if os.path.exists(CACHE_FILE_PATH):
        with open(CACHE_FILE_PATH, 'r') as file:
            return int(file.read().strip())  # Read the cached length from the file
    return 0  # Return 0 if cache file does not exist (initial state)

def update_cache(length):
    """Update the cache file with the new length."""
    with open(CACHE_FILE_PATH, 'w') as file:
        file.write(str(length))  # Write the new length to the cache file

def should_retrain():
    """Check if retraining is needed based on CSV data."""
    
    # Try to load the datasets from CSV files
    try:
        # Read the datasets
        data_th = pd.read_csv('data_th.csv')  # Data from the 'data_th.csv' file
        data_mb = pd.read_csv('data_mb.csv')  # Data from the 'data_mb.csv' file
        
        # Check if either dataset is empty, exit if so
        if data_th.empty or data_mb.empty:
            print("One or both datasets are empty. Exiting.")
            return False  # No retraining needed if datasets are empty
        
        # Get current total length of both datasets combined
        current_length = len(data_th) + len(data_mb)
        
        # Retrieve the cached length from the previous check
        cached_length = get_cached_length()
        
        # If the current data length is greater than the cached length, retraining is needed
        if current_length > cached_length:
            update_cache(current_length)  # Update cache with new data length
            return True  # Retraining needed due to new data
        
        return False  # No retraining needed, data hasn't changed
    
    except FileNotFoundError:
        # If the CSV files don't exist, retraining is needed
        print("CSV files not found. Retraining will be required.")
        return True  # Retraining needed if files are not found
