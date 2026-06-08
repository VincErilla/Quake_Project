#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 acc;

void setup(void) {
  Serial.begin(115200);
  
  // Attempt to intialize MPU6050
  Serial.println("Initializing MPU6050...");
  if (!acc.begin()) {
    Serial.println("Failed to intialize.");
    while (1);
  } Serial.println("Initialization successful.");

  acc.setAccelerometerRange(MPU6050_RANGE_8_G);
}

void loop() {
  sensors_event_t a, g, t;
  acc.getEvent(&a, &g, &t);

  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  delay(500);
}
