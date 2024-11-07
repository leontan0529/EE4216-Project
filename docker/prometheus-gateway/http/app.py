from flask import Flask, request
import numpy as np
import cv2
import os
from datetime import datetime

app = Flask(__name__)

UPLOAD_FOLDER = '/prometheus_data'
UPLOAD_PARENT = 'int'
GENERIC_FOLDER = '/images'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(GENERIC_FOLDER, exist_ok=True)

def save_img(img):
    # Generate a directory path with the current date and hour
    current_time = datetime.now()
    date_path = current_time.strftime('%Y-%m-%d/%H')
    save_path = os.path.join(UPLOAD_FOLDER, date_path, UPLOAD_PARENT)
    os.makedirs(save_path, exist_ok=True)

    # Save the file with its original filename in the created directory
    filename = current_time.strftime('%Y%m%d_%H%M%S') + '.jpeg'
    cv2.imwrite(os.path.join(save_path, filename), img)
    print("Image Saved as {filename}.") # debug

def grafana_img(img):
    # Save the file with a generic filename in the created directory
    save_dupe_path = os.path.join(GENERIC_FOLDER)
    os.makedirs(save_dupe_path, exist_ok=True)
    imagename = 'image.jpeg'
    cv2.imwrite(os.path.join(save_dupe_path, imagename), img)
    print("Updated Grafana Image.") # debug

@app.route('/upload', methods=['POST','GET'])
def upload():
    received = request
    img = None
    if received.files:
        print(received.files['imageFile'])
        #convert string of image data to uint8
        file  = received.files['imageFile']
        nparr = np.fromstring(file.read(), np.uint8)

        # decode image
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        save_img(img)
        grafana_img(img)

        return "[SUCCESS] Image Received", 200
    else:
        return "[FAILED] Image Not Received", 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=1884)