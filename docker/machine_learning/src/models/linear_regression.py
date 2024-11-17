import pandas as pd
from sklearn.linear_model import LinearRegression
from sklearn.metrics import mean_squared_error, r2_score
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
import pickle
import sys

def train_linear_regression(X, y, model_save_path):
    
    # Split the dataset into training and testing sets.
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

    # Normalize features.
    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)

    # Train Linear Regression Model.
    lr_model = LinearRegression()
    lr_model.fit(X_train_scaled, y_train)

    # Predictions and evaluation for Linear Regression Model.
    y_pred_lr = lr_model.predict(X_test_scaled)

    mse_value = mean_squared_error(y_test, y_pred_lr)  # Calculate MSE
    r2_value = r2_score(y_test, y_pred_lr)  # Calculate R^2 Score

    # Save the trained model to a file.
    with open(model_save_path, 'wb') as file:
        pickle.dump(lr_model, file)

    log_file_path = model_save_path + '.log'
    with open(log_file_path, 'w') as log_file:
        log_file.write(f'Mean Squared Error: {mse_value}\n')
        log_file.write(f'R^2 Score: {r2_value}\n')

    print(f"Model saved to {model_save_path}")

