from flask import Blueprint, request, jsonify

its_bp = Blueprint('its', __name__)

@its_bp.route('/', methods=['POST'])
def its_data():
    data = request.get_json()
    if not data:
        return jsonify({"error": "No data received"}), 400
    print(f"Received ITS data: {data}")
    return jsonify({"message": "ITS data received successfully"}), 200
