#include "esp_camera.h"
#include <WiFi.h>

// --- WiFi credentials ---
const char* ssid = "Sankalp's S21+";
const char* password = "sanky@07";

// --- PIR pin ---
#define PIR_PIN 14

// --- AI Thinker ESP32-CAM pin map ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

WiFiServer server(80);
uint8_t* latestImage = NULL;
size_t latestImageLen = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  delay(1000);

  Serial.println("⏳ Letting PIR stabilize...");
  delay(15000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.print("\n✅ Connected: ");
  Serial.println(WiFi.localIP());

  // --- Camera configuration ---
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 15;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 18;
    config.fb_count = 1;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("❌ Camera init failed");
    return;
  }

  server.begin();
  Serial.println("📡 Web server started");
}

void loop() {
  static unsigned long lastTrigger = 0;

  // --- Motion Detection ---
  if (digitalRead(PIR_PIN) == HIGH && millis() - lastTrigger > 5000) {
    lastTrigger = millis();
    Serial.println("🚨 Motion detected! Capturing snapshot...");

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("❌ Failed to get frame buffer");
    } else {
      if (latestImage) free(latestImage);
      latestImage = (uint8_t*)malloc(fb->len);

      memcpy(latestImage, fb->buf, fb->len);
      latestImageLen = fb->len;
      Serial.printf("📸 Snapshot captured (%u bytes)\n", fb->len);
      esp_camera_fb_return(fb);
    }
  }

  // --- Handle Web Requests ---
  WiFiClient client = server.available();
  if (!client) return;

  String req = client.readStringUntil('\r');
  client.readStringUntil('\n');
  client.flush();

  if (req.indexOf("GET /stream") != -1) {
    // --- Livestream Endpoint ---
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println();

    while (client.connected()) {
      camera_fb_t* fb = esp_camera_fb_get();
      if (!fb) continue;

      client.println("--frame");
      client.println("Content-Type: image/jpeg");
      client.print("Content-Length: "); client.println(fb->len);
      client.println();
      client.write(fb->buf, fb->len);
      client.println();

      esp_camera_fb_return(fb);
      delay(150);  // Throttle stream rate
    }

  } else if (req.indexOf("GET /latest.jpg") != -1) {
    // --- Latest Snapshot Endpoint ---
    if (latestImage && latestImageLen > 0) {
      client.println("HTTP/1.1 200 OK");
      client.println("Cache-Control: no-store");  // 🚫 Prevent caching
      client.println("Content-Type: image/jpeg");
      client.print("Content-Length: "); client.println(latestImageLen);
      client.println();
      client.write(latestImage, latestImageLen);
    } else {
      client.println("HTTP/1.1 404 Not Found");
      client.println();
    }

  } else {
    // --- Main Webpage ---
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Livestream + PIR</title>
  <style>
    body { margin: 0; background: #000; }
    .main { position: relative; display: inline-block; }
    .overlay {
      position: absolute;
      bottom: 10px;
      right: 10px;
      border: 2px solid white;
      width: 160px;
      height: 120px;
      z-index: 10;
    }
  </style>
</head>
<body>
  <div class="main">
    <img src="/stream" />
    <img src="/latest.jpg" class="overlay" id="motion">
  </div>
  <script>
    setInterval(() => {
      const img = document.getElementById('motion');
      img.src = '/latest.jpg?t=' + new Date().getTime();  // 🔄 Force refresh
    }, 3000);
  </script>
</body>
</html>
)rawliteral");
  }

  client.stop();
}




