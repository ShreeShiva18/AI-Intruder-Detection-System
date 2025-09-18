import os
import requests
import smtplib
from deepface import DeepFace
from PIL import Image
import numpy as np
from email.message import EmailMessage

# --- Configuration ---
ESP32_CAM_URL = "http://192.168.36.87/latest.jpg"

KNOWN_FACES_DIR = "known_faces"
INTRUDER_IMG = "intruder.jpg"
EMAIL_SENDER = "shreeshiva182@gmail.com"
EMAIL_RECEIVER = "shreeshivamn04@gmail.com"
EMAIL_PASSWORD = "SSSMMMNNN18@@@"  # Use Gmail App Password

# --- Capture intruder image from ESP32-CAM ---
def capture_intruder_image():
    try:
        response = requests.get(ESP32_CAM_URL, timeout=5)
        if response.status_code == 200 and response.headers["Content-Type"].startswith("image/"):
            with open(INTRUDER_IMG, "wb") as f:
                f.write(response.content)
            print("✅ Image saved as intruder.jpg")
            return True
        else:
            print("❌ Failed to capture image or invalid response")
            return False
    except requests.exceptions.RequestException as e:
        print("❌ Error connecting to ESP32-CAM:", e)
        return False

# --- Send alert email with image attached ---
def send_alert_email():
    msg = EmailMessage()
    msg.set_content("⚠️ Unknown person detected by the camera!")
    msg["Subject"] = "ALERT: Intruder Detected"
    msg["From"] = EMAIL_SENDER
    msg["To"] = EMAIL_RECEIVER

    try:
        with open(INTRUDER_IMG, "rb") as f:
            msg.add_attachment(f.read(), maintype="image", subtype="jpeg", filename="intruder.jpg")

        with smtplib.SMTP_SSL("smtp.gmail.com", 465) as server:
            server.login(EMAIL_SENDER, EMAIL_PASSWORD)
            server.send_message(msg)
        print("✅ Email sent successfully.")
    except Exception as e:
        print("❌ Failed to send email:", e)

# --- Compare intruder with known faces using DeepFace ---
def recognize_face():
    try:
        for person in os.listdir(KNOWN_FACES_DIR):
            person_folder = os.path.join(KNOWN_FACES_DIR, person)

            for file in os.listdir(person_folder):
                known_face_path = os.path.join(person_folder, file)
                if not file.lower().endswith((".jpg", ".jpeg", ".png")):
                    continue

                print(f"🔍 Comparing with {known_face_path}...")
                try:
                    result = DeepFace.verify(img1_path=INTRUDER_IMG, img2_path=known_face_path, enforce_detection=False)
                    if result["verified"]:
                        print(f"✅ Match found! Known person: {person}")
                        return
                except Exception as e:
                    print(f"⚠️ DeepFace error with {known_face_path}: {e}")

        print("⚠️ No match found — unknown person detected!")
        send_alert_email()

    except Exception as e:
        print("❌ Error during face recognition:", e)

# --- Main ---
if capture_intruder_image():
    recognize_face()
