
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

  Serial.println("LIS331HH test!");

  //if (!lis.begin_SPI(LIS331HH_CS)) {
//  if (!lis.begin_SPI(LIS331HH_CS, LIS331HH_SCK, LIS331HH_MISO, LIS331HH_MOSI)) {
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
  // Serial.println("LIS331HH found!");

 lis1.setRange(LIS331HH_RANGE_6_G);   // 6, 12, or 24 G
  Serial.print("Range set to: ");
  switch (lis1.getRange()) {
    case LIS331HH_RANGE_6_G: Serial.println("6 g"); break;
    case LIS331HH_RANGE_12_G: Serial.println("12 g"); break;
    case LIS331HH_RANGE_24_G: Serial.println("24 g"); break;
  }
  lis2.setRange(LIS331HH_RANGE_6_G);   // 6, 12, or 24 G
  Serial.print("Range set to: ");
  switch (lis2.getRange()) {
    case LIS331HH_RANGE_6_G: Serial.println("6 g"); break;
    case LIS331HH_RANGE_12_G: Serial.println("12 g"); break;
    case LIS331HH_RANGE_24_G: Serial.println("24 g"); break;
  }
  lis3.setRange(LIS331HH_RANGE_6_G);   // 6, 12, or 24 G
  Serial.print("Range set to: ");
  switch (lis3.getRange()) {
    case LIS331HH_RANGE_6_G: Serial.println("6 g"); break;
    case LIS331HH_RANGE_12_G: Serial.println("12 g"); break;
    case LIS331HH_RANGE_24_G: Serial.println("24 g"); break;
  }
  // lis.setDataRate(LIS331_DATARATE_50_HZ);
  Serial.println("Accel 1 Data rate set to: ");
  switch (lis1.getDataRate()) {

    case LIS331_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
    case LIS331_DATARATE_50_HZ: Serial.println("50 Hz"); break;
    case LIS331_DATARATE_100_HZ: Serial.println("100 Hz"); break;
    case LIS331_DATARATE_400_HZ: Serial.println("400 Hz"); break;
    case LIS331_DATARATE_1000_HZ: Serial.println("1000 Hz"); break;
    case LIS331_DATARATE_LOWPOWER_0_5_HZ: Serial.println("0.5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_1_HZ: Serial.println("1 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_2_HZ: Serial.println("2 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_5_HZ: Serial.println("5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_10_HZ: Serial.println("10 Hz Low Power"); break;

  }
  Serial.println("Accel 2 Data rate set to: ");
  switch (lis2.getDataRate()) {

    case LIS331_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
    case LIS331_DATARATE_50_HZ: Serial.println("50 Hz"); break;
    case LIS331_DATARATE_100_HZ: Serial.println("100 Hz"); break;
    case LIS331_DATARATE_400_HZ: Serial.println("400 Hz"); break;
    case LIS331_DATARATE_1000_HZ: Serial.println("1000 Hz"); break;
    case LIS331_DATARATE_LOWPOWER_0_5_HZ: Serial.println("0.5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_1_HZ: Serial.println("1 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_2_HZ: Serial.println("2 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_5_HZ: Serial.println("5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_10_HZ: Serial.println("10 Hz Low Power"); break;

  }
  Serial.println("Accel 3 Data rate set to: ");
  switch (lis3.getDataRate()) {

    case LIS331_DATARATE_POWERDOWN: Serial.println("Powered Down"); break;
    case LIS331_DATARATE_50_HZ: Serial.println("50 Hz"); break;
    case LIS331_DATARATE_100_HZ: Serial.println("100 Hz"); break;
    case LIS331_DATARATE_400_HZ: Serial.println("400 Hz"); break;
    case LIS331_DATARATE_1000_HZ: Serial.println("1000 Hz"); break;
    case LIS331_DATARATE_LOWPOWER_0_5_HZ: Serial.println("0.5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_1_HZ: Serial.println("1 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_2_HZ: Serial.println("2 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_5_HZ: Serial.println("5 Hz Low Power"); break;
    case LIS331_DATARATE_LOWPOWER_10_HZ: Serial.println("10 Hz Low Power"); break;

  }
}

void loop() {
  /* Get a new sensor event, normalized */
  sensors_event_t event1, event2, event3, event4;
  lis1.getEvent(&event1);
  lis2.getEvent(&event2);
  lis3.getEvent(&event3);
  accel.getEvent(&event4);

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("Accelerometer 1: ");
  Serial.print("\t\tX: "); Serial.print(event1.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event1.acceleration.y);
  Serial.print(" \tZ: "); Serial.print(event1.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.print("Accelerometer 2: ");
  Serial.print("\t\tX: "); Serial.print(event2.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event2.acceleration.y);
  Serial.print(" \tZ: "); Serial.print(event2.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.print("Accelerometer 3: ");
  Serial.print("\t\tX: "); Serial.print(event3.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event3.acceleration.y);
  Serial.print(" \tZ: "); Serial.print(event3.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.print("200g Accelerometer: ");
  Serial.print("\t\tX: "); Serial.print(event4.acceleration.x); Serial.print("  ");
  Serial.print( "\tY: "); Serial.print(event4.acceleration.y); Serial.print("  ");
  Serial.print(" \tZ: "); Serial.print(event4.acceleration.z); Serial.print("  ");
  Serial.println(" m/s^2 ");
  Serial.println();

  delay(1000);
}