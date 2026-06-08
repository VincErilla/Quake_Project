
// Basic demo for accelerometer readings from Adafruit LIS331HH

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL375.h>

#define ADXL375_SCK 13
#define ADXL375_MISO 12
#define ADXL375_MOSI 11
#define ADXL375_CS 10

// Used for software SPI
#define LIS331HH_SCK 5
#define LIS331HH_MISO 21
#define LIS331HH_MOSI 19
// Used for hardware & software SPI
#define LIS331HH_CS 15

Adafruit_LIS331HH lis1 = Adafruit_LIS331HH();
Adafruit_LIS331HH lis2 = Adafruit_LIS331HH();
Adafruit_LIS331HH lis3 = Adafruit_LIS331HH();

Adafruit_ADXL375 accel = Adafruit_ADXL375(12345);


void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  if (! lis1.begin_I2C()) {   // change this to 0x19 for alternative i2c address
    Serial.println("Accelerometer 1 Couldnt start");
    while (1) yield();
  }
  Serial.println("Accelerometer 1 connected!");
  if (! lis2.begin_I2C()) {   // change this to 0x19 for alternative i2c address
    Serial.println("Accelerometer 2 Couldnt start");
    while (1) yield();
  }
  Serial.println("Accelerometer 2 connected!");
  if (!lis3.begin_SPI(LIS331HH_CS, LIS331HH_SCK, LIS331HH_MISO, LIS331HH_MOSI)) {
    Serial.println("Accelerometer 3 Couldnt start");
    while (1) yield();
  }
  Serial.println("Accelerometer 3 connected!");
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL375 ... check your connections */
    Serial.println("Ooops, no ADXL375 detected ... Check your wiring!");
    while(1);
  }
  Serial.println("200g accelerometer connected!");

  lis1.setRange(LIS331HH_RANGE_12_G);   // 6, 12, or 24 G
  lis2.setRange(LIS331HH_RANGE_12_G);   // 6, 12, or 24 G
  lis3.setRange(LIS331HH_RANGE_12_G);   // 6, 12, or 24 G

  
  
}

void loop() {
  float x1, y1, z1;  // Sensor 1 (LIS331HH)
  float x2, y2, z2;  // Sensor 2 (LIS331HH)
  float x3, y3, z3;  // Sensor 3 (LIS331HH)
  float x4, y4, z4;  // Sensor 4 (ADXL375)
  float t;           // Time

  sensors_event_t event1, event2, event3, event4;
  lis1.getEvent(&event1);
  lis2.getEvent(&event2);
  lis3.getEvent(&event3);
  accel.getEvent(&event4);

  // Read data from all sensors
  x1 = (event1.acceleration.x);
  y1 = (event1.acceleration.y);
  z1 = (event1.acceleration.z);

  x2 = (event2.acceleration.x);
  y2 = (event2.acceleration.y);
  z2 = (event2.acceleration.z);

  x3 = (event3.acceleration.x);
  y3 = (event3.acceleration.y);
  z3 = (event3.acceleration.z);

  x4 = (event4.acceleration.x);
  y4 = (event4.acceleration.y);
  z4 = (event4.acceleration.z);

  t = (millis() / 1000.0);
  // Send data over Serial in the format: t, s1_x, s1_y, s1_z, s2_x, ...
  Serial.print(t);  // Timestamp in seconds
  Serial.print(", ");
  Serial.print(x1); Serial.print(", "); Serial.print(y1); Serial.print(", "); Serial.print(z1); Serial.print(", ");
  Serial.print(x2); Serial.print(", "); Serial.print(y2); Serial.print(", "); Serial.print(z2); Serial.print(", ");
  Serial.print(x3); Serial.print(", "); Serial.print(y3); Serial.print(", "); Serial.print(z3); Serial.print(", ");
  Serial.print(x4); Serial.print(", "); Serial.print(y4); Serial.print(", "); Serial.println(z4);

  delay(1000);  // Adjust sampling rate (e.g., 10 samples/second)
}