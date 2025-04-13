import os
from pathlib import Path
from typing import Tuple, Optional
from werkzeug.utils import secure_filename


class CloudException(Exception):
    pass


def allowed_file(filename: str, app) -> bool:
    """Check if the Uploaded File is Allowed Based on Extension."""
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in app.config['ALLOWED_EXTENSIONS']


def save_file_locally(file, base_path: str, app) -> Optional[str]:
    """
    Save the Uploaded File locally.
    Returns tuple (success_flag, file_path or None).
    """
    try:
        filename = secure_filename(file.filename)
        file_path = os.path.join(base_path, filename)
        file.save(file_path)
        return file_path
    
    except Exception as e:
        return None

def save_file_on_cloud(file_path: str, cloud) -> str:
    """
    Upload File from Local Path to Cloud (Supabase Storage).
    Returns the File URL/path on Cloud if Successful.
    """
    with open(file_path, 'rb') as f:
        file_data = f.read()

    filename = Path(file_path).name
    storage_response = cloud.storage.from_('firmwares').upload(filename, file_data)
    if not storage_response:
        raise CloudException("Unable to Upload File on Cloud.")

    return storage_response.path


def add_firmware_for_update(file_path: str, version: str, cloud) -> bool:
    """
    Upload File and Insert Metadata about the Uploaded File into the 'FirmwareManager' Table.
    Returns True if operation was successful.
    """
    cloud_url = save_file_on_cloud(file_path, cloud)
    response = cloud.table('FirmwareManager').insert({
        'version': version,
        'file_url': cloud_url
    }).execute()
    
    if not response:
        raise CloudException("Unable to Create Firmware Management Record.")
    
    return len(response.data) > 0 if hasattr(response, 'data') else False
