#!/bin/bash

# Define the directory where the model files are saved
MODEL_DIR="src/model.pkl/"

# Function to extract metrics (Mean Squared Error and R^2 Score) from a log file
extract_metrics() {
    local log_file=$1
    local mse
    local r2

    # Check if the log file exists
    if [ -f "$log_file" ]; then
        # Extract the Mean Squared Error (MSE) and R^2 Score from the log file
        mse=$(grep "Mean Squared Error" "$log_file" | awk '{print $NF}')
        r2=$(grep "R^2 Score" "$log_file" | awk '{print $NF}')
        echo "$mse:$r2"  # Return the MSE and R^2 as a colon-separated string
    else
        # If the log file doesn't exist, print an error and exit
        echo "Log file not found: $log_file"
        exit 1
    fi
}

# Function to find the best model based on the metrics (MSE and R^2) from log files
find_best_model() {
    local model_prefix=$1  # Prefix to identify the model type (e.g., trained_RF, trained_LR)
    local best_model=""     # Variable to store the path of the best model
    local best_metrics="999999:0"  # Initialize with high MSE (bad performance) and low R^2 (bad performance)

    # Loop through all models in the directory that match the model prefix
    for model_path in ${MODEL_DIR}${model_prefix}_model_*.pkl; do
        if [ -f "$model_path" ]; then
            # Extract the metrics from the corresponding log file
            metrics=$(extract_metrics "${model_path}.log")
            mse=$(echo $metrics | cut -d':' -f1)  # Extract MSE from the metrics
            r2=$(echo $metrics | cut -d':' -f2)   # Extract R^2 from the metrics

            # Compare the extracted metrics with the best metrics so far
            # If the current model has a lower MSE and higher R^2, update the best model
            if (( $(echo "$mse < ${best_metrics%%:*}" | bc -l) )) && (( $(echo "$r2 > ${best_metrics##*:}" | bc -l) )); then
                best_model=$model_path  # Update the best model path
                best_metrics="$mse:$r2" # Update the best metrics
            fi
        fi
    done

    echo "$best_model"  # Return the path of the best model found
}

# Find the best Random Forest model
BEST_RF_MODEL_PATH=$(find_best_model "trained_RF")  # Call the function to find the best Random Forest model
echo "Best Random Forest model: $BEST_RF_MODEL_PATH"

# Find the best Linear Regression model
BEST_LR_MODEL_PATH=$(find_best_model "trained_LR")  # Call the function to find the best Linear Regression model
echo "Best Linear Regression model: $BEST_LR_MODEL_PATH"

# Remove all other Random Forest models except the best one
for model_path in ${MODEL_DIR}trained_RF_model_*.pkl; do
    if [ "$model_path" != "$BEST_RF_MODEL_PATH" ]; then
        # If the current model is not the best model, remove it along with its log file
        echo "Removing old Random Forest model: $model_path"
        rm "$model_path" "${model_path}.log"
    fi
done

# Remove all other Linear Regression models except the best one
for model_path in ${MODEL_DIR}trained_LR_model_*.pkl; do
    if [ "$model_path" != "$BEST_LR_MODEL_PATH" ]; then
        # If the current model is not the best model, remove it along with its log file
        echo "Removing old Linear Regression model: $model_path"
        rm "$model_path" "${model_path}.log"
    fi
done

# Update the paths of the best models in the constants.py file
python3 - <<EOF
import os

# Define the path to the constants.py file and the new values for the best models
constants_path = 'src/constants.py'
best_rf_model_path = r"$BEST_RF_MODEL_PATH"
best_lr_model_path = r"$BEST_LR_MODEL_PATH"

# Read the current contents of constants.py
with open(constants_path, 'r') as f:
    lines = f.readlines()

# Write the updated lines to constants.py
with open(constants_path, 'w') as f:
    for line in lines:
        # Update the BEST_RF_MODEL_PATH and BEST_LR_MODEL_PATH values in the file
        if line.startswith('BEST_RF_MODEL_PATH'):
            f.write(f'BEST_RF_MODEL_PATH = r"{best_rf_model_path}"\n')
        elif line.startswith('BEST_LR_MODEL_PATH'):
            f.write(f'BEST_LR_MODEL_PATH = r"{best_lr_model_path}"\n')
        else:
            # Write the unchanged lines as is
            f.write(line)

print("Updated constants.py with best model paths.")
EOF

echo "Model comparison and cleanup completed."  # Final message indicating the completion of the process
