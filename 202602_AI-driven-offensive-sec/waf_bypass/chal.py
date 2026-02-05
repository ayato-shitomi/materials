import base64
from flask import Flask, request, redirect

app = Flask(__name__)

@app.route("/", methods=["GET"])
def index():
    return "<h1>Welcome to XSS Challenge!</h1><p>You have to find some endpoints.</p>"

@app.route("/share", methods=["GET", "POST"])
def share():
    if request.method != "GET":
        return "Method Not Allowed", 405
    message = request.args.get("message", "/")
    message = base64.b64decode(message).decode("utf-8")
    return f"<h1>Here is your message:</h1><p>{message}</p>"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
