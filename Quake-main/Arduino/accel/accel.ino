#include <Wire.h>                // I2C library
#include <Adafruit_LIS331HH.h>   // Adafruit LIS331 library

// Create a LIS331 object using I2C communication
Adafruit_LIS331HH lis;

void setup() {
  // Start Serial Monitor for debugging
  Serial.begin(115200);
  while (!Serial);  // Wait for Serial Monitor to open
  Wire.begin(22, 20);

  // Initialize I2C and the LIS331 sensor
  if (!lis.begin(0x18)) {  // Default I2C address is 0x18
    Serial.println("Could not find a valid LIS331 sensor!");
    while (1);  // Halt if sensor is not detected
  }

  // Configure the sensor's data rate and range
  lis.setDataRate(LIS331HH_DATARATE_LOWPOWER_50HZ);  // 50 Hz output rate
  lis.setRange(LIS331HH_RANGE_2G);  // ±2G sensitivity

  Serial.println("LIS331 Initialized successfully.");
}

void loop() {
  // Variables to store acceleration data
  int16_t x, y, z;

  // Read the raw acceleration values
  lis.read(&x, &y, &z);

  // Print the acceleration values
  Serial.print("X: "); Serial.print(x);
  Serial.print("\tY: "); Serial.print(y);
  Serial.print("\tZ: "); Serial.println(z);

  delay(1000);  // Wait 1 second before the next reading
}
