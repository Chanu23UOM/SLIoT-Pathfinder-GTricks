#include <WiFi.h>
#include <WebServer.h>

// Replace with your actual Wi-Fi credentials
const char* ssid = "SDG";
const char* password = "13131313";

// Initialize the web server on default HTTP port 80
WebServer server(80);

// The built-in LED is on pin 2 for the DOIT ESP32 V1
const int ledPin = 2; 

// Function to generate the HTML interface
String getHTML() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
  html += ".button { background-color: #008CBA; border: none; color: white; padding: 16px 40px;";
  html += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; border-radius: 8px;}";
  html += ".button-off {background-color: #555555;}</style></head>";
  html += "<body><h1>ESP32 Control Panel</h1>";
  html += "<p>Onboard Blue LED</p>";
  html += "<p><a href=\"/on\"><button class=\"button\">Turn ON</button></a></p>";
  html += "<p><a href=\"/off\"><button class=\"button button-off\">Turn OFF</button></a></p>";
  html += "</body></html>";
  return html;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize the LED pin as an output and turn it off initially
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Wi-Fi Connected!");
  Serial.print("Access your server at this IP Address: http://");
  Serial.println(WiFi.localIP());

  // Define how the server handles different web requests (routes)
  server.on("/", []() {
    server.send(200, "text/html", getHTML()); // Load main page
  });
  
  server.on("/on", []() {
    digitalWrite(ledPin, HIGH);               // Turn LED on
    server.send(200, "text/html", getHTML()); // Reload page
  });
  
  server.on("/off", []() {
    digitalWrite(ledPin, LOW);                // Turn LED off
    server.send(200, "text/html", getHTML()); // Reload page
  });

  // Start the server
  server.begin();
  Serial.println("HTTP server started!");
}

void loop() {
  // Listen for incoming client requests from your browser
  server.handleClient(); 
}