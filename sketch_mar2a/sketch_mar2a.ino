#define LED_PIN 2

void setup() {
  // Setup the pin as an output
  pinMode(LED_PIN, OUTPUT);
  // Start the serial monitor for debugging
  Serial.begin(115200);
  Serial.println("ESP32 Boot Successful! Starting Blink...");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);  // Turn LED ON
  Serial.println("LED ON");
  delay(1000);                  // Wait 1 second
  
  digitalWrite(LED_PIN, LOW);   // Turn LED OFF
  Serial.println("LED OFF");
  delay(1000);                  // Wait 1 second
}