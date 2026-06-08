#include <Wire.h> 
#include <SPI.h>  
#include <RTClib.h>
#include <SD.h>   

// RTC variables
RTC_DS3231 rtc;
DateTime now;

// SD variables
const int chipSelectSD = 10;     
File accFile;  
int fileNum = 0;
String fileName = "";

void setup() { 
  Serial.begin(9600);  

  // Initialize the SD reader (HW-125)
  Serial.println("Initializing SD reader...");
  if (!SD.begin(chipSelectSD)) {
    Serial.println("Initialization of SD reader failed.\n"); 
    while (1); 
  }
  Serial.println("Initialization done.\n");

  // Initialize RTC (DS3231)
  Serial.println("Initializing RTC...");
  if (!rtc.begin()) {
    Serial.println("Initialization of RTC failed.\n");
    while (1); 
  }
  Serial.println("Initialization done.");
}

void loop() {  
  now = rtc.now();   

  // Creates date string, adding 0s when necessary 
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

    // Run number can be 0-99 for a given date
    if (fileNum < 10) {
      fileName += "0" + String(fileNum) + ".csv";
    } else {
      fileName += "" + String(fileNum) + ".csv";
    }
    break;
  }
  Serial.println("File Name: " + fileName);

  // **Doesn't like anything not a number in the file name.
  accFile = SD.open(fileName, FILE_WRITE); 
  if (accFile) {
    accFile.close();
    while(1);
  } else {
    Serial.println("Could not access file");
    while(1);
  }  
}
