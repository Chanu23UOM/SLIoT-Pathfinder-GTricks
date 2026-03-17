#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for Serial console to open
  }

  Serial.println("\n--- MPU6050 Isolated Hardware Diagnostic ---");

  // 1. Start the I2C bus on your custom pins
  Wire.begin(32, 33); 
  Serial.println("I2C Bus started on Pins 32 (SDA) and 33 (SCL).");

  // 2. Attempt to connect to the sensor
  if (!mpu.begin()) {
    Serial.println("\n[ERROR] Failed to find MPU6050 chip!");
    Serial.println("1. Check your jumper wires (are they loose?).");
    Serial.println("2. Ensure SDA is on D32 and SCL is on D33.");
    Serial.println("3. Try moving the VCC wire from 3.3V to the VIN / 5V pin.");
    while (1) {
      delay(10); // Freeze the board here if it fails
    }
  }
  
  Serial.println("[SUCCESS] MPU6050 Found and Initialized!\n");
  
  // Configure the sensor for basic readings
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);
}

void loop() {
  // Get new sensor events with the readings
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print the Accelerometer data
  Serial.print("Accel X: ");
  Serial.print(a.acceleration.x, 2);
  Serial.print(" | Y: ");
  Serial.print(a.acceleration.y, 2);
  Serial.print(" | Z: ");
  Serial.print(a.acceleration.z, 2);
  Serial.print(" m/s^2");

  // Print the Temperature data
  Serial.print("  ---  Temp: ");
  Serial.print(temp.temperature, 1);
  Serial.println(" C");

  delay(500); // Wait half a second before reading again
}