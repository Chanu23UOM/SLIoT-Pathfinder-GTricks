#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your actual Wi-Fi credentials
const char* ssid = "YOUR_WIFI_NETWORK_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// The server we want to test communication with
const char* serverName = "http://example.com";

void setup() {
  Serial.begin(115200);
  delay(1000); // Give the Serial Monitor a second to catch up

  // 1. Initiate Wi-Fi Connection
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Wait in a loop until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Wi-Fi Connected Successfully!");
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());

  // 2. Test Internet Access (HTTP GET Request)
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    Serial.print("Attempting to reach: ");
    Serial.println(serverName);

    // Initialize the HTTP client with the server URL
    http.begin(serverName);
    
    // Send the GET request
    int httpResponseCode = http.GET();

    // Check the response
    if (httpResponseCode > 0) {
      Serial.print("✅ Server reached! HTTP Response code: ");
      Serial.println(httpResponseCode); // 200 means "OK"
    } else {
      Serial.print("❌ Error reaching server. Code: ");
      Serial.println(httpResponseCode);
    }
    
    // Free up resources
    http.end(); 
  }
}

void loop() {
  // We leave the loop empty so it only pings the server once on startup.
}