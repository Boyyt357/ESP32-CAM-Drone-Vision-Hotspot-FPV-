#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <esp_camera.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>

// Camera pin definitions for AI-Thinker ESP32-CAM
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

// Network settings
const char* ssid = "ESP32-CAM-FPV";
const char* password = "fpv123456";

WebServer server(80);

// Performance tracking variables
unsigned long lastFrameTime = 0;
unsigned long lastStatsTime = 0;
unsigned long frameCount = 0;
unsigned long totalBytes = 0;
float currentFPS = 0;
float currentBitrate = 0;
float rangePercent = 100;
int currentQuality = 10;
int packetLossCount = 0;
int successfulFrames = 0;

// Adaptive quality parameters
const int MIN_QUALITY = 5;   // Best quality (larger files)
const int MAX_QUALITY = 30;  // Worst quality (smaller files)
const int MIN_FPS = 10;
const int MAX_FPS = 30;
int targetFPS = 25;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32-CAM Long Range FPV System ===");
  
  // Initialize camera
  if (!initCamera()) {
    Serial.println("Camera init failed!");
    return;
  }
  
  // Setup Wi-Fi AP for maximum range
  setupWiFiAP();
  
  // Setup web server routes
  setupWebServer();
  
  Serial.println("System ready!");
  Serial.println("Connect to Wi-Fi: " + String(ssid));
  Serial.println("Video stream: http://192.168.4.1/mjpeg");
  Serial.println("Stats API: http://192.168.4.1/stats");
  Serial.println("HTML Interface: http://192.168.4.1/");
}

bool initCamera() {
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
  config.xclk_freq_hz = 30000000;  // 30MHz for stability
  config.pixel_format = PIXFORMAT_JPEG;
  
  // VGA resolution for good balance of quality/performance
  config.frame_size = FRAMESIZE_VGA;  // 640x480
  config.jpeg_quality = currentQuality;
  config.fb_count = 2;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }
  
  // Get camera sensor and apply settings
  sensor_t* s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0-No Effect, 1-Negative, 2-Grayscale, 3-Red Tint, 4-Green Tint, 5-Blue Tint, 6-Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
    s->set_aec2(s, 0);           // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);       // -2 to 2
    s->set_aec_value(s, 300);    // 0 to 1200
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_vflip(s, 0);          // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  }
  
  return true;
}

void setupWiFiAP() {
  // Configure for maximum range with 802.11b
  WiFi.mode(WIFI_AP);
  
  // Set to 802.11b mode for maximum range
  esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B);
  
  // Set channel 1 and 20MHz bandwidth for best range
  esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);
  
  // Maximum transmit power
  esp_wifi_set_max_tx_power(78); // 19.5 dBm (78 * 0.25)
  
  WiFi.softAP(ssid, password, 1, 0, 1); // Channel 1, no hidden, max 1 connection
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP IP address: " + IP.toString());
  Serial.println("Wi-Fi Mode: 802.11b (Long Range)");
  Serial.println("Channel: 1 | Bandwidth: 20MHz | TX Power: Max");
}

void setupWebServer() {
  // Serve the HTML interface
  server.on("/", HTTP_GET, handleRoot);
  
  // MJPEG video stream
  server.on("/mjpeg", HTTP_GET, handleMJPEG);
  
  // Stats JSON API with CORS support
  server.on("/stats", HTTP_GET, handleStats);
  server.on("/stats", HTTP_OPTIONS, handleStatsOptions);
  
  server.begin();
  Serial.println("Web server started");
}

void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>ESP32-CAM FPV</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { margin: 0; padding: 0; background: #000; font-family: Arial, sans-serif; overflow: hidden; }
        #container { position: relative; width: 100vw; height: 100vh; }
        #video { width: 100%; height: 100%; object-fit: contain; }
        #hud { position: absolute; top: 10px; left: 10px; color: #00ff00; background: rgba(0,0,0,0.7); 
               padding: 10px; border-radius: 5px; font-family: 'Courier New', monospace; font-size: 14px; 
               text-shadow: 1px 1px 2px #000; }
        #signal-bar { position: absolute; top: 10px; right: 10px; width: 100px; height: 20px; 
                      border: 2px solid #00ff00; background: rgba(0,0,0,0.7); }
        #signal-fill { height: 100%; background: linear-gradient(to right, #ff0000, #ffff00, #00ff00); 
                       transition: width 0.3s ease; }
        .status { margin: 2px 0; }
        .good { color: #00ff00; }
        .warning { color: #ffff00; }
        .critical { color: #ff0000; }
    </style>
</head>
<body>
    <div id="container">
        <img id="video" src="/mjpeg" alt="FPV Stream">
        <div id="hud">
            <div class="status">ESP32-CAM FPV</div>
            <div class="status">FPS: <span id="fps">--</span></div>
            <div class="status">Quality: <span id="quality">--</span></div>
            <div class="status">Bitrate: <span id="bitrate">-- kB/s</span></div>
            <div class="status">Range: <span id="range">--%</span></div>
            <div class="status">Mode: 802.11b Long Range</div>
        </div>
        <div id="signal-bar">
            <div id="signal-fill" style="width: 100%;"></div>
        </div>
    </div>

    <script>
        function updateStats() {
            fetch('/stats')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('fps').textContent = data.fps.toFixed(1);
                    document.getElementById('quality').textContent = data.quality;
                    document.getElementById('bitrate').textContent = data.bitrate.toFixed(1);
                    document.getElementById('range').textContent = data.range.toFixed(0);
                    
                    // Update signal bar
                    const signalFill = document.getElementById('signal-fill');
                    signalFill.style.width = data.range + '%';
                    
                    // Color coding based on performance
                    const fpsElement = document.getElementById('fps');
                    const rangeElement = document.getElementById('range');
                    
                    // FPS status
                    if (data.fps >= 20) fpsElement.className = 'good';
                    else if (data.fps >= 15) fpsElement.className = 'warning';
                    else fpsElement.className = 'critical';
                    
                    // Range status
                    if (data.range >= 70) rangeElement.className = 'good';
                    else if (data.range >= 40) rangeElement.className = 'warning';
                    else rangeElement.className = 'critical';
                })
                .catch(error => {
                    console.log('Stats update failed:', error);
                    // Show disconnected status
                    document.getElementById('fps').textContent = '--';
                    document.getElementById('range').textContent = '--';
                });
        }
        
        // Update stats every 500ms
        setInterval(updateStats, 500);
        updateStats(); // Initial update
        
        // Handle video errors
        document.getElementById('video').addEventListener('error', function() {
            console.log('Video stream error - attempting reconnect...');
            setTimeout(() => {
                this.src = '/mjpeg?' + Date.now(); // Cache bust
            }, 1000);
        });
    </script>
</body>
</html>
)";
  server.send(200, "text/html", html);
}

void handleMJPEG() {
  WiFiClient client = server.client();
  if (!client) return;
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(response);
  
  unsigned long streamStartTime = millis();
  frameCount = 0;
  
  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      packetLossCount++;
      Serial.println("Camera capture failed");
      delay(10);
      continue;
    }
    
    // Send MJPEG frame
    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %u\r\n\r\n", fb->len);
    
    size_t written = client.write(fb->buf, fb->len);
    client.print("\r\n");
    
    if (written == fb->len) {
      successfulFrames++;
      totalBytes += fb->len;
    } else {
      packetLossCount++;
    }
    
    esp_camera_fb_return(fb);
    frameCount++;
    
    // Calculate and update performance metrics
    updatePerformanceMetrics();
    
    // Adaptive quality control
    adaptiveQualityControl();
    
    // Respect target FPS
    unsigned long frameTime = 1000 / targetFPS;
    unsigned long elapsed = millis() - lastFrameTime;
    if (elapsed < frameTime) {
      delay(frameTime - elapsed);
    }
    lastFrameTime = millis();
  }
}

void handleStats() {
  DynamicJsonDocument doc(300);
  doc["fps"] = currentFPS;
  doc["bitrate"] = currentBitrate;
  doc["range"] = rangePercent;
  doc["quality"] = currentQuality;
  doc["uptime"] = millis() / 1000;
  doc["frameCount"] = frameCount;
  doc["totalFrames"] = successfulFrames;
  doc["lossCount"] = packetLossCount;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Add CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "application/json", jsonString);
}

// Add OPTIONS handler for CORS preflight
void handleStatsOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(200, "text/plain", "");
}

void updatePerformanceMetrics() {
  unsigned long currentTime = millis();
  
  // Update every second
  if (currentTime - lastStatsTime >= 1000) {
    float elapsed = (currentTime - lastStatsTime) / 1000.0;
    
    // Calculate FPS and bitrate
    currentFPS = frameCount / elapsed;
    currentBitrate = (totalBytes / elapsed) / 1024.0; // kB/s
    
    // Calculate range percentage based on multiple factors
    float packetLossRate = 0;
    if (packetLossCount + successfulFrames > 0) {
      packetLossRate = (float)packetLossCount / (packetLossCount + successfulFrames);
    }
    
    float fpsEfficiency = (targetFPS > 0) ? (currentFPS / targetFPS) : 1.0;
    fpsEfficiency = constrain(fpsEfficiency, 0, 1);
    
    // Calculate range based on performance metrics
    rangePercent = (1.0 - packetLossRate) * fpsEfficiency * 100.0;
    rangePercent = constrain(rangePercent, 0, 100);
    
    // If we have very low metrics, simulate based on quality
    if (currentFPS < 1 && currentBitrate < 1) {
      rangePercent = map(currentQuality, MIN_QUALITY, MAX_QUALITY, 90, 30);
    }
    
    // Print stats to serial
    Serial.printf("FPS: %.1f | Bitrate: %.1f kB/s | Range: %.0f%% | Quality: %d | Loss: %d | Success: %d\n", 
                  currentFPS, currentBitrate, rangePercent, currentQuality, packetLossCount, successfulFrames);
    
    // Reset counters
    frameCount = 0;
    totalBytes = 0;
    lastStatsTime = currentTime;
    
    // Reset loss counters periodically to prevent overflow
    if (packetLossCount > 50 || successfulFrames > 500) {
      packetLossCount /= 2;
      successfulFrames /= 2;
    }
  }
}

void adaptiveQualityControl() {
  static unsigned long lastAdaptTime = 0;
  unsigned long currentTime = millis();
  
  // Adapt every 2 seconds
  if (currentTime - lastAdaptTime < 2000) return;
  lastAdaptTime = currentTime;
  
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return;
  
  // Quality adjustment based on performance
  if (rangePercent < 60 || currentFPS < (targetFPS * 0.8)) {
    // Poor performance - reduce quality/increase compression
    currentQuality = min(currentQuality + 2, MAX_QUALITY);
    targetFPS = max(targetFPS - 1, MIN_FPS);
    Serial.println("Reducing quality due to poor link");
  } 
  else if (rangePercent > 80 && currentFPS >= targetFPS) {
    // Good performance - improve quality
    if (currentQuality > MIN_QUALITY) {
      currentQuality = max(currentQuality - 1, MIN_QUALITY);
      Serial.println("Improving quality due to good link");
    } else if (targetFPS < MAX_FPS) {
      targetFPS = min(targetFPS + 1, MAX_FPS);
      Serial.println("Increasing target FPS");
    }
  }
  
  // Apply new quality setting
  s->set_quality(s, currentQuality);
}

void loop() {
  server.handleClient();
  delay(1); // Small delay for stability
}
