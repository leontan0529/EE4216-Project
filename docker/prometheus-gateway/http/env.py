from flask import Blueprint, request, jsonify

env_bp = Blueprint('env', __name__)

@env_bp.route('/', methods=['POST'])
def env_data():
    data = request.get_json()
    if not data:
        return jsonify({"error": "No data received"}), 400
    print(f"Received env data: {data}")
    return jsonify({"message": "Env data received successfully"}), 200
