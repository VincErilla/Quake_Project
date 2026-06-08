// Basic demo for accelerometer readings from Adafruit LIS331HH

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS331HH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL375.h>
#include <SD.h>
#include <SD_MMC.h> // For SDIO support

#define SD_MMC_CMD_PIN '33' // CMD pin
#define SD_MMC_D0_PIN '27'   // Data pin 0
#define SD_MMC_D1_PIN '32'   // Data pin 1
#define SD_MMC_D2_PIN '12'  // Data pin 2
#define SD_MMC_D3_PIN '13'  // Data pin 3

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

File dataFile; // File object for SD card writing

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10); // will pause Zero, Leonardo, etc until serial console opens

  // Initialize SD card using SD_MMC for SDIO
  if (!SD_MMC.begin(SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Create or open the data log file
  dataFile = SD_MMC.open("/datalog.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error opening datalog.txt");
  }

  // Initialize accelerometers
  if (!lis1.begin_I2C()) {
    Serial.println("Accelerometer 1 Couldn't start");
    while (1) yield();
  }
  Serial.println("Accelerometer 1 connected!");
  
  if (!lis2.begin_I2C()) {
    Serial.println("Accelerometer 2 Couldn't start");
    while (1) yield();
  }
  Serial.println("Accelerometer 2 connected!");

  if (!lis3.begin_SPI(LIS331HH_CS, LIS331HH_SCK, LIS331HH_MISO, LIS331HH_MOSI)) {
    Serial.println("Accelerometer 3 Couldn't start");
    while (1) yield();
  }
  Serial.println("Accelerometer 3 connected!");

  if (!accel.begin()) {
    Serial.println("Ooops, no ADXL375 detected ... Check your wiring!");
    while (1);
  }
  Serial.println("200g accelerometer connected!");

  lis1.setRange(LIS331HH_RANGE_12_G); // 6, 12, or 24 G
  lis2.setRange(LIS331HH_RANGE_12_G); // 6, 12, or 24 G
  lis3.setRange(LIS331HH_RANGE_12_G); // 6, 12, or 24 G
}

void loop() {
  float x1, y1, z1;  // Sensor 1 (LIS331HH)
  float x2, y2, z2;  // Sensor 2 (LIS331HH)
  float x3, y3, z3;  // Sensor 3 (LIS331HH)
  float x4, y4, z4;  // Sensor 4 (ADXL375)
  float rms1, rms2, rms3, rms4; // RMS values for each sensor
  float t;           // Time

  sensors_event_t event1, event2, event3, event4;
  lis1.getEvent(&event1);
  lis2.getEvent(&event2);
  lis3.getEvent(&event3);
  accel.getEvent(&event4);

  // Read data from all sensors
  x1 = event1.acceleration.x;
  y1 = event1.acceleration.y;
  z1 = event1.acceleration.z;

  x2 = event2.acceleration.x;
  y2 = event2.acceleration.y;
  z2 = event2.acceleration.z;

  x3 = event3.acceleration.x;
  y3 = event3.acceleration.y;
  z3 = event3.acceleration.z;

  x4 = event4.acceleration.x;
  y4 = event4.acceleration.y;
  z4 = event4.acceleration.z;

  // Calculate RMS value for each sensor
  rms1 = sqrt((x1 * x1 + y1 * y1 + z1 * z1) / 3.0);
  rms2 = sqrt((x2 * x2 + y2 * y2 + z2 * z2) / 3.0);
  rms3 = sqrt((x3 * x3 + y3 * y3 + z3 * z3) / 3.0);
  rms4 = sqrt((x4 * x4 + y4 * y4 + z4 * z4) / 3.0);

  t = (millis() / 1000.0);
  
  // Send data over Serial in the format: t, s1_x, s1_y, s1_z, s2_x, ...
  Serial.print(t);  // Timestamp in seconds
  Serial.print(", ");
  Serial.print(x1); Serial.print(", "); Serial.print(y1); Serial.print(", "); Serial.print(z1); Serial.print(", ");
  Serial.print(x2); Serial.print(", "); Serial.print(y2); Serial.print(", "); Serial.print(z2); Serial.print(", ");
  Serial.print(x3); Serial.print(", "); Serial.print(y3); Serial.print(", "); Serial.print(z3); Serial.print(", ");
  Serial.print(x4); Serial.print(", "); Serial.print(y4); Serial.print(", "); Serial.println(z4);
  
  // Write data to SD card
  if (dataFile) {
    dataFile.print(t); // Timestamp
    dataFile.print(", ");
    dataFile.print(x1); dataFile.print(", "); dataFile.print(y1); dataFile.print(", "); dataFile.print(z1); dataFile.print(", ");
    dataFile.print(rms1); dataFile.print(", "); // Write RMS for Sensor 1
    dataFile.print(x2); dataFile.print(", "); dataFile.print(y2); dataFile.print(", "); dataFile.print(z2); dataFile.print(", ");
    dataFile.print(rms2); dataFile.print(", "); // Write RMS for Sensor 2
    dataFile.print(x3); dataFile.print(", "); dataFile.print(y3); dataFile.print(", "); dataFile.print(z3); dataFile.print(", ");
    dataFile.print(rms3); dataFile.print(", "); // Write RMS for Sensor 3
    dataFile.print(x4); dataFile.print(", "); dataFile.print(y4); dataFile.print(", "); dataFile.println(z4);
    dataFile.print(rms4); // Write RMS for Sensor 4
    dataFile.flush();  // Ensure data is written to the SD card
  } else {
    Serial.println("Data file is not open.");
  }

  delay(1000);  // Adjust sampling rate (e.g., 10 samples/second)
}
