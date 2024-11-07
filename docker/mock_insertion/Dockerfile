# Start with an official Python base image
FROM python:3.9-slim

# Set working directory
WORKDIR /app

# Copy requirements.txt to install dependencies
COPY . /app

# Run the Prometheus exporter
CMD ["python", "app.py"]