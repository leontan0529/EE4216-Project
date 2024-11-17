import pandas as pd
import os

def load_log_file(file_path):
    """Load a log file and return it as a DataFrame."""
    try:
        # Read the log file into a pandas DataFrame, assuming whitespace-separated values
        # 'sep=r'\s+'': regular expression to match one or more whitespace characters
        # 'header=None' specifies no header row in the log file
        # 'names=["timestamp", "value"]' assigns column names to the DataFrame
        df = pd.read_csv(file_path, sep=r'\s+', header=None, names=["timestamp", "value"])
        
        # Convert the timestamp to datetime format (currently commented out)
        #df['timestamp'] = pd.to_datetime(df['timestamp'], format="%Y-%m-%d %H:%M:%S")
        
        return df  # Return the DataFrame with loaded log data
    except Exception as e:
        # If an error occurs (e.g., file not found or format issue), print the error message
        print(f"Error reading file {file_path}: {e}")
        return None  # Return None in case of error

def load_data_from_folders(base_folder):
    """Load data from the folders (lux, gas, hum, tem)."""
    
    # Initialize variables to store DataFrames for each log file
    gas_df, hum_df, lux_df, tem_df = None, None, None, None

    # Loop through the date folders (e.g., '2024-11-01') in the base folder
    for date_folder in os.listdir(base_folder):
        # Construct the full path for each date folder
        date_path = os.path.join(base_folder, date_folder)
        
        # Check if it's a directory and if the folder name matches the format YYYY-MM-DD
        if os.path.isdir(date_path) and len(date_folder) == 10 and date_folder[4] == '-' and date_folder[7] == '-':
            print(date_path)  # Print the path for debugging

        if os.path.isdir(date_path):  # Ensure it's a directory before continuing
            # Loop through the time subfolders (e.g., '00', '01', '10') inside the date folder
            for time_folder in os.listdir(date_path):
                time_path = os.path.join(date_path, time_folder)
                print(time_path)  # Print the path for debugging

                if os.path.isdir(time_path):  # Check if it's a directory
                    # Loop through each expected log file (gas, hum, lux, tem) in the time folder
                    for log_file in ['gas/gas.log', 'hum/hum.log', 'lux/lux.log', 'tem/tem.log']:
                        # Construct the full path to each log file
                        file_path = os.path.join(time_path, log_file)
                        print(log_file)  # Print the name of the log file for debugging
                        print(file_path)  # Print the full file path for debugging

                        # Check if the file exists and is a regular file (not a directory)
                        if os.path.isfile(file_path):
                            # Load the data from the log file and store it in the corresponding DataFrame
                            df = load_log_file(file_path)

                            # Assign the loaded DataFrame to the appropriate variable based on the log file
                            if log_file == 'gas/gas.log':
                                gas_df = df
                            elif log_file == 'hum/hum.log':
                                hum_df = df
                            elif log_file == 'lux/lux.log':
                                lux_df = df
                            elif log_file == 'tem/tem.log':
                                tem_df = df

    # Return the loaded DataFrames (could be None if no valid data was found)
    return gas_df, hum_df, lux_df, tem_df
