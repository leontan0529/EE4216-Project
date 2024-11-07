# Start with an official Python base image
FROM python:3.9-slim

# Set working directory
WORKDIR /app

# Copy requirements.txt to install dependencies
COPY . /app

# Install required Python packages
RUN pip install -r requirements.txt

# Run the Prometheus exporter
CMD ["python", "app.py"]