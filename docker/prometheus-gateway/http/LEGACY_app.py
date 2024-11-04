from flask import Flask, request
import os
from datetime import datetime

app = Flask(__name__)

UPLOAD_FOLDER = '/prometheus_data'
UPLOAD_PARENT = 'int'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route('/upload', methods=['POST'])
def upload_file():
    if 'image' not in request.files:
        return 'No file part', 400
    file = request.files['image']
    if file.filename == '':
        return 'No selected file', 400
    if file:
        # Generate a directory path with the current date and hour
        current_time = datetime.now()
        date_path = current_time.strftime('%Y-%m-%d/%H')
        save_path = os.path.join(UPLOAD_FOLDER, date_path, UPLOAD_PARENT)
        os.makedirs(save_path, exist_ok=True)
        
        # Save the file with its original filename in the created directory
        filename = current_time.strftime('%Y%m%d_%H%M%S') + '.jpeg'
        file.save(os.path.join(save_path, filename))
        return 'File successfully uploaded', 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=1884)
