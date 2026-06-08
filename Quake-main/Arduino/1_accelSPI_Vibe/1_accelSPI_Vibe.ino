#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <ADXL345_WE.h>

// Updated configurations to utilize hardware SPI pins and Pin 13 for CS
#define CS_PIN 13

bool spiMode = true;
// Uses the standard default global SPI instance (SCK=5, MI=21, MO=19)
ADXL345_WE accel = ADXL345_WE(&SPI, CS_PIN, spiMode);

unsigned long lastSampleTime = 0;
unsigned long startTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // Wait for Serial Monitor to connect
  
  // Explicitly force CS Pin 13 into an output state and set it HIGH 
  // to ensure it doesn't float during hardware initialization
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  
  // Start the default hardware SPI bus
  SPI.begin();
  
  if (!accel.init()) {
    Serial.println("ADXL345 initialization failed!");
    while (1);
  }

  // Hardware configurations (1600 Hz ODR, +/-16g range)
  accel.setDataRate(ADXL345_DATA_RATE_1600);
  accel.setRange(ADXL345_RANGE_16G);

  Serial.println("Initialization complete. Streaming uncalibrated library data...");
  Serial.println("Time(s),X(g),Y(g),Z(g)"); 
  
  startTime = micros();
}

void loop() {
  unsigned long now = micros() - startTime;

  // MASTER TIMER (2000 microseconds = 500 Hz sampling)
  if (now - lastSampleTime >= 2000) {
    lastSampleTime += 2000;

    xyzFloat e;
    accel.getGValues(&e); 

    // Calculate elapsed time in seconds
    float t = now / 1000000.0;

    // Stream out data: Time, X, Y, Z
    Serial.printf("%.4f,%.4f,%.4f,%.4f\n", t, e.x, e.y, e.z);
  }
}