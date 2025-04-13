import os
from flask import Flask, request, jsonify, render_template
from supabase import create_client, Client
from utils.saver import save_file_locally, allowed_file, CloudException, add_firmware_for_update
from dotenv import load_dotenv
from flask import render_template


load_dotenv()

app = Flask(__name__)

app.config['UPLOAD_FOLDER'] = 'uploads'
app.config['ALLOWED_EXTENSIONS'] = {'bin'}

# Check if the Uploads Directory Exists
os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)

# Supabase Setup
url = os.getenv('SUPABASE_URL')
key = os.getenv('SUPABASE_KEY')
supabase: Client = create_client(url, key)


@app.route('/upload_file', methods=['POST'])
def upload_new_firmware():
    if 'file' not in request.files:
        return jsonify({'error': 'No file part'}), 400

    file = request.files['file']
    version = request.form.get('version')

    if file.filename == '':
        return jsonify({'error': 'No selected file'}), 400

    if not allowed_file(file.filename, app):
        return jsonify({'error': 'Invalid file format'}), 400

    try:
        file_path = save_file_locally(file, app.config["UPLOAD_FOLDER"], app)
        if not file_path:
            jsonify("Unable to Store File Locally")
            
        add_firmware_for_update(file_path, version, supabase)
        print("Saved on Cloud")
        return jsonify({'message': 'File Uploaded Successfully'}), 200

    except CloudException as e:
        return jsonify({'error': str(e)}), 500
    
    except Exception as e:
        return jsonify({'error': f'Unexpected Error: {str(e)}'}), 500



@app.route('/get-data', methods=['GET'])
def get_sensory_data():
    pass


@app.route('/get-fdata', methods=['GET'])
def get_filtered_data(filters):
    pass


# > Add a TTL on Local File Storage


########
# Pages
########
@app.route("/")
def index():
    """Dashboard Page"""
    pass


@app.route("/")
def analytics():
    """Analytics Page"""
    # Use these Columns
    # Supabase Table Name: SensorData
    # Columns: temperature, humidity, created_at
    # Better to Create a Realtime Subscription on Insertions
    # Add a Timeout on Subscription, should expire in 10 minutes of Inactivity from Frontend
    pass


@app.route('/upload')
def upload():
    return render_template('upload.html')



if __name__ == '__main__':
    app.run(debug=True)
