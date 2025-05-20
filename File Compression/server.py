import os
import subprocess
from flask import Flask, render_template, request, send_file, jsonify

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
EXE_PATH = os.path.join(BASE_DIR, "huffmancomp.exe")
UPLOAD_FOLDER = os.path.join(BASE_DIR, "uploads")
PROCESSED_FOLDER = os.path.join(BASE_DIR, "processed")

os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(PROCESSED_FOLDER, exist_ok=True)

@app.route("/", methods=["GET", "POST"])
def index():
    if request.method == "POST":
        if not os.path.exists(EXE_PATH):
            return jsonify({"error": "huffmancomp.exe not found!"}), 500

        if "file" not in request.files:
            return jsonify({"error": "No file uploaded!"}), 400

        uploaded_file = request.files["file"]
        if uploaded_file.filename == "":
            return jsonify({"error": "No file selected!"}), 400

        input_file_path = os.path.join(UPLOAD_FOLDER, uploaded_file.filename)
        uploaded_file.save(input_file_path)

        action = request.form.get("action", "compress")  
        output_extension = ".txt"  
        output_file_path = os.path.join(PROCESSED_FOLDER, f"{os.path.splitext(uploaded_file.filename)[0]}_output{output_extension}")

        try:
            
            result = subprocess.run(
                [EXE_PATH, input_file_path, output_file_path, action],
                check=True,
                cwd=BASE_DIR,
                capture_output=True,
                text=True
            )

         
            output_message = result.stdout.strip() if result.stdout else "Operation completed successfully."

            return jsonify({
                "success": True,
                "message": output_message,
                "output_file": f"/download/{os.path.basename(output_file_path)}",
                "file_path": output_file_path
            })

        except subprocess.CalledProcessError as e:
            return jsonify({"error": f"Error running Huffman compressor: {e.stderr}"}), 500

    return render_template("index.html")

@app.route("/download/<filename>")
def download(filename):
    file_path = os.path.join(PROCESSED_FOLDER, filename)

    if os.path.exists(file_path):
        return send_file(file_path, as_attachment=True)

    return jsonify({"error": "File not found!"}), 404

if __name__ == "__main__":
    app.run(debug=True)