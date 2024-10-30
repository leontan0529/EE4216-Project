from flask import Flask
from env import env_bp
from its import its_bp

app = Flask(__name__)
app.register_blueprint(env_bp, url_prefix='/env')
app.register_blueprint(its_bp, url_prefix='/its')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)