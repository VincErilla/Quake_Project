#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>              
#include <SD.h>               
#include <RTClib.h>  
#include <math.h>

// Creates object(s) for MPU6050 accelerometer
Adafruit_MPU6050 acc;
double xData;
double yData;
double zData;

// Variables for SD-card reader
const int chipSelectSD = 10;      // Sets pin for SD-Reader chip select
File accFile;                     // File object for data output
String fileName = "";             // File name will populate with date and run number
int fileNum = 0;                  // For if you do more than one test in a day

// Variables for RTC (Real-Time Clock)
RTC_DS3231 rtc;
DateTime now;
int day = 0;
int month = 0;
int year = 0;

// Variables for time keeping
unsigned long startTime;
unsigned long nowTime;
double formatTime;
double runTime = 10.0;

int ledPin = 4;

void setup() {
  Serial.begin(115200);
  
  // Attempt to intialize MPU6050
  Serial.println("Initializing MPU6050...");
  if (!acc.begin()) {
    Serial.println("Failed to intialize.");
    while (1);
  } 
  acc.setAccelerometerRange(MPU6050_RANGE_16_G);
  acc.setFilterBandwidth(MPU6050_BAND_260_HZ);
  Serial.println("Initialization successful.");

  // Initialize the SD reader (HW-125)
  Serial.println("Initializing SD reader (HW-125)...");
  if (!SD.begin(chipSelectSD)) {
    Serial.println("Initialization of SD reader (HW-125) failed.\n"); 
    while (1); 
  }
  Serial.println("Initialization done.\n");  

  // Initialize RTC (DS3231)
  Serial.println("Initializing RTC (DS3231)...");
  if (!rtc.begin()) {
    Serial.println("Initialization of RTC (DS3231) failed.\n");
    while (1); 
  }
  Serial.println("Initialization done.");
  Serial.println("---------------------------------------");
}

void loop() {
  // **At the moment, starts at power on then stalls after taking data
  // Creates file name based on day in ddmmyy format    
  now = rtc.now();    
  if (now.month() < 10) {
    fileName += "0";
  } fileName += String(now.month());
  if (now.day() < 10) {
    fileName += "0";
  } fileName += String(now.day());
  fileName += String(now.year() % 100);
    
  // Check for next file name for given date and add .csv extention    
  while (1) {
    if (fileNum < 10) {
      if (SD.exists(fileName + "0" + String(fileNum) + ".csv")) {
        fileNum++;
        continue;
      }
    } else {
      if (SD.exists(fileName + String(fileNum) + ".csv")) {
        fileNum++;
        continue;
      }
    }
    if (fileNum < 10) {
      fileName += "0" + String(fileNum) + ".csv";
    } else {
      fileName += String(fileNum) + ".csv";
    }
    break;
  } 

  accFile = SD.open(fileName, FILE_WRITE);    

  if (accFile) {
    digitalWrite(ledPin, HIGH); // LED should be lit up while the SD card is taking data.
    accFile.println("time,xaxis,yaxis,zaxis"); // Adds first line for table headers    
      
    Serial.println(fileName); 
    Serial.println("Taking Data.");

    startTime = micros();

    while(1) {
      // **Possible to only take acceleration?
      sensors_event_t a, g, t;
      acc.getEvent(&a, &g, &t);
      nowTime = micros();

      xData = a.acceleration.x;
      yData = a.acceleration.y;
      zData = a.acceleration.z;
      formatTime = double(nowTime - startTime) / pow(10.0, 6.0);
      
      accFile.print(formatTime, 4);
      accFile.print(",");
      accFile.print(xData);
      accFile.print(",");
      accFile.print(yData);
      accFile.print(",");
      accFile.println(zData);

      if (formatTime >= runTime) {
        accFile.close();
        Serial.println("Done taking data.");
        digitalWrite(ledPin, LOW);
        while(1);
      }
    } 
  } else {
    // There was an error accessing file
    Serial.println("Couldn't write to file.\n");
    while (1);
  }
}
