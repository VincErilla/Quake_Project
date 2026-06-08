#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <ADXL345_WE.h>

// SPI pins for custom bus
#define HSPI_MISO 36
#define HSPI_MOSI 15
#define HSPI_SCK 14
#define CS1 32
#define CS2 20

bool spi = true;
SPIClass spiAccel(HSPI);
ADXL345_WE accel1 = ADXL345_WE(&spiAccel, CS1, spi);
ADXL345_WE accel2 = ADXL345_WE(&spiAccel, CS2, spi);


void setup() {
  Serial.begin(115200);

  spiAccel.begin(HSPI_SCK, HSPI_MISO, HSPI_MOSI, -1);
  if (!accel1.init()) {
    Serial.println("Accel 1 not detected");
    while (1);
  }
  if (!accel2.init()) {
    Serial.println("Accel 2 not detected");
    while (1);
  }

  accel1.setDataRate(ADXL345_DATA_RATE_1600);
  accel2.setDataRate(ADXL345_DATA_RATE_1600);
  accel1.setRange(ADXL345_RANGE_16G);
  accel2.setRange(ADXL345_RANGE_16G);

}

void loop() {
  unsigned long now = micros();
  
  xyzFloat e3, e4;
  accel1.getGValues(&e3);
  accel2.getGValues(&e4);


  Serial.print("1,x:"); Serial.print(e3.x, 3);
  Serial.print(",y:"); Serial.print(e3.y, 3);
  Serial.print(",z:"); Serial.println(e3.z, 3);

  Serial.print("2,x:"); Serial.print(e4.x, 3);
  Serial.print(",y:"); Serial.print(e4.y, 3);
  Serial.print(",z:"); Serial.println(e4.z, 3);

  delay(100);
}
