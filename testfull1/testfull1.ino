#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <LoRa.h>

// --- LoRa Pin Definitions (ESP32) ---
#define LORA_SS    15 
#define LORA_RST   16 
#define LORA_DIO0  0  

// --- Network Settings ---
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer server(80);

// --- Sensor Objects & Hardware Flags ---
Adafruit_MPU6050 mpu;
Adafruit_BME280 bme; 

bool hasMPU = false;
bool hasBME = false;
bool hasLoRa = false;
bool simulatedAlertActive = false; // Used to trigger fake alerts from the web UI

// --- DSP Variables ---
float filtered_Z = 0.0;
float baseline_Z = 0.0;
const float ALPHA = 0.2; 
const float IMPACT_THRESHOLD = 1.5; 
unsigned long lastSensorRead = 0;
unsigned long lastAlertTime = 0;

// --- HTML Interface (Stored in PROGMEM) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Pathfinder Smart Post</title>
  <style>
    body { font-family: sans-serif; background-color: #121212; color: #fff; text-align: center; padding: 20px; margin: 0; }
    h1 { color: #00E676; margin-bottom: 5px; }
    .grid-container { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin-top: 20px; }
    .card { background: #1E1E1E; padding: 20px; border-radius: 12px; border: 1px solid #333; }
    .data { font-size: 32px; font-weight: bold; margin: 10px 0 0 0; color: #29B6F6; }
    .alert-card { grid-column: 1 / -1; background: #3E2723; border: 1px solid #FF5252; transition: 0.3s; }
    .btn { width: 100%; padding: 20px; color: white; border: none; border-radius: 8px; font-size: 18px; font-weight: bold; cursor: pointer; margin-top: 15px; }
    #sosBtn { background-color: #D32F2F; }
    #simBtn { background-color: #F57C00; }
  </style>
</head>
<body>
  <h1>Pathfinder Node</h1>
  <p>Offline Wilderness Guide</p>
  
  <div class="grid-container">
    <div class="card"><h2>Temp</h2><p class="data"><span id="temp">--</span> &deg;C</p></div>
    <div class="card"><h2>Humidity</h2><p class="data"><span id="hum">--</span> %</p></div>
    <div class="card" style="grid-column: 1 / -1;"><h2>Pressure</h2><p class="data"><span id="pres">--</span> hPa</p></div>
    <div class="card alert-card" id="status-card"><h2>Area Status</h2><p class="data" id="status" style="color:#00E676;">SAFE</p></div>
  </div>
  
  <button id="sosBtn" class="btn">🚨 SEND EMERGENCY SOS</button>
  <button id="simBtn" class="btn">🐘 SIMULATE SEISMIC ALERT</button>

  <script>
    // Fetch sensor data every 2 seconds
    setInterval(function() {
      fetch('/sensordata').then(response => response.json()).then(data => {
          document.getElementById('temp').innerText = data.temperature;
          document.getElementById('hum').innerText = data.humidity;
          document.getElementById('pres').innerText = data.pressure;
          
          if(data.alert == "1") {
             document.getElementById('status-card').style.background = '#4A148C';
             document.getElementById('status').style.color = '#E040FB';
             document.getElementById('status').innerText = "VIBRATION ALERT!";
          } else {
             document.getElementById('status-card').style.background = '#3E2723';
             document.getElementById('status').style.color = '#00E676';
             document.getElementById('status').innerText = "SAFE";
          }
      });
    }, 2000);

    // SOS Button Logic
    document.getElementById('sosBtn').addEventListener('click', function() {
      if(confirm("Send Emergency SOS?")) {
        fetch('/trigger_sos', { method: 'POST' }).then(response => {
            if(response.ok) {
              this.innerText = "✅ SOS SENT!";
              this.style.backgroundColor = "#388E3C";
              setTimeout(() => { this.innerText = "🚨 SEND EMERGENCY SOS"; this.style.backgroundColor = "#D32F2F"; }, 5000);
            }
        });
      }
    });

    // Simulate Vibration Alert Logic
    document.getElementById('simBtn').addEventListener('click', function() {
        fetch('/simulate_alert', { method: 'POST' });
    });
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- Starting Pathfinder Node ---");
  
  // 1. Initialize Sensors & Set Flags
  if (!mpu.begin()) { 
    Serial.println("⚠️ MPU6050 missing! Running in MOCK SEISMIC mode."); 
    hasMPU = false;
  } else { 
    mpu.setFilterBandwidth(MPU6050_BAND_44_HZ); 
    hasMPU = true;
    
    // Get baseline gravity only if connected
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    baseline_Z = a.acceleration.z;
  }
  
  if (!bme.begin(0x76)) { 
    Serial.println("⚠️ BME280 missing! Running in MOCK WEATHER mode."); 
    hasBME = false;
  } else {
    hasBME = true;
  }

  // 2. Initialize LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) { 
    Serial.println("⚠️ LoRa failed/missing! TX will only print to Serial."); 
    hasLoRa = false;
  } else {
    hasLoRa = true;
  }

  // 3. Setup Wi-Fi AP & Captive Portal
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Pathfinder-Net"); 
  dnsServer.start(DNS_PORT, "*", apIP);

  // 4. Web Server Routes
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", index_html); });
  
  server.on("/sensordata", HTTP_GET, []() {
    String json = "{";
    // Send real data if available, otherwise generate mock data
    json += "\"temperature\":\"" + String(hasBME ? bme.readTemperature() : (24.0 + random(0, 30)/10.0), 1) + "\",";
    json += "\"humidity\":\"" + String(hasBME ? bme.readHumidity() : (60.0 + random(0, 10)), 0) + "\",";
    json += "\"pressure\":\"" + String(hasBME ? (bme.readPressure() / 100.0F) : (1010.0 + random(-5, 5)), 1) + "\",";
    
    // Determine alert status (real vibration OR simulated web button)
    bool isAlerting = (abs(filtered_Z) > IMPACT_THRESHOLD) || simulatedAlertActive;
    json += "\"alert\":\"" + String(isAlerting ? 1 : 0) + "\"";
    json += "}";
    
    server.send(200, "application/json", json);
    
    // Turn off simulated alert after it has been read by the web UI once
    if (simulatedAlertActive) simulatedAlertActive = false; 
  });

  server.on("/trigger_sos", HTTP_POST, []() {
    Serial.println("🚨 VIRTUAL SOS BUTTON CLICKED ON WEB UI!");
    sendLoRaAlert("VIRTUAL_SOS");
    server.send(200, "text/plain", "SOS Transmitted");
  });

  server.on("/simulate_alert", HTTP_POST, []() {
    Serial.println("🐘 SEISMIC ALERT SIMULATED FROM WEB UI!");
    simulatedAlertActive = true;
    sendLoRaAlert("SIMULATED_SEISMIC_ALERT");
    server.send(200, "text/plain", "Alert Triggered");
  });

  server.onNotFound([]() {
    server.sendHeader("Location", String("http://") + apIP.toString(), true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("✅ System Ready. Connect your phone to 'Pathfinder-Net'");
}

void loop() {
  // Handle Web and DNS requests constantly
  dnsServer.processNextRequest();
  server.handleClient();

  // Read real MPU sensor data ONLY if it is connected
  if (hasMPU && (millis() - lastSensorRead > 10)) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    float raw_Z = a.acceleration.z - baseline_Z;
    filtered_Z = (ALPHA * raw_Z) + ((1.0 - ALPHA) * filtered_Z);

    if (abs(filtered_Z) > IMPACT_THRESHOLD && (millis() - lastAlertTime > 5000)) {
      Serial.println("🚨 REAL SEISMIC IMPACT DETECTED!");
      sendLoRaAlert("SEISMIC_ALERT");
      lastAlertTime = millis();
    }
    lastSensorRead = millis();
  }
}

void sendLoRaAlert(String alertType) {
  if (hasLoRa) {
    LoRa.beginPacket();
    LoRa.print("NODE_01, ALERT: ");
    LoRa.print(alertType);
    LoRa.endPacket();
    Serial.println("📡 ACTUAL LoRa Packet Transmitted: " + alertType);
  } else {
    // If no LoRa module, just print to Serial to prove the logic fired
    Serial.println("📡 MOCK LoRa TX (Hardware Missing): " + alertType);
  }
}