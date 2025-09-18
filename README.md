# AI-Intruder-Detection-System
An AI-powered intruder detection system using ESP32-CAM, PIR sensor, and Python (DeepFace) for real-time face recognition and automated email alerts. 
This is a security system that combines motion sensing, image capture, and facial recognition to detect intruders in real time. The project uses an ESP32-CAM module with a PIR sensor to capture images upon motion detection. A Python application then analyzes the captured images using the DeepFace library, verifies identities against a known database, and sends automated email alerts if an unknown person is detected

🚀 Features

Motion detection using PIR sensor
Real-time image capture with ESP32-CAM (OV2640 camera)
Face recognition powered by DeepFace
Automatic email alerts with intruder images via SMTP
Local web server for live surveillance feed
Compact, low-cost, and scalable system design

🛠️ Hardware Used

ESP32-CAM (OV2640 camera)
PIR motion sensor
FTDI programmer (for ESP32-CAM flashing)
Power supply (5V)

🧑‍💻 Software & Libraries

Arduino IDE (ESP32-CAM programming)
Python (image retrieval + face recognition)
DeepFace (face verification)
SMTP (smtplib) for email alerts

⚙️ How It Works

PIR sensor detects motion → triggers ESP32-CAM
ESP32-CAM captures image and hosts it on a web server
Python script retrieves the image and runs DeepFace verification
If face matches → authorized access logged
If face is unknown → alert email sent with image attachment

📊 Results

Motion detection range: 5–7 meters
Face recognition accuracy: ~80–85%
Alert delivery time: 5–10 seconds

📌 Applications

Home security
Office/workplace surveillance
Retail shops & warehouses
Smart door/lock systems
Banks, ATMs, and school campuses
