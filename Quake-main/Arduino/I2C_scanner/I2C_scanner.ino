#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial);  // Wait for Serial Monitor to open
  Serial.println("\nI2C Scanner");

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at address 0x");
      Serial.println(address, HEX);
    }
  }
}

void loop() {
  delay(5000);  // Scan every 5 seconds
}
