// TO-DO
//-------
// More accurate time
// Accelerometer
//    -Eliminate noise (Appears above 100Hz per documentation)
//    -Try lower Hz for noise checking
//    -Auto-Calibration

#include <Wire.h>             
#include <Adafruit_Sensor.h>  
#include <Adafruit_ADXL375.h> 
#include <SPI.h>              
#include <SD.h>               
#include <RTClib.h>           

// Creates an accelerometer object
Adafruit_ADXL375 accel = Adafruit_ADXL375(12345);
bool readAcc = false;             // For reading accelerometer

// Variables for pushbutton, not needed in final design
const int buttonPin = 2;          // Pin number for button (subject to change)
int buttonState = 0;              // Variable for reading pushbutton status

// Variables for LED
const int ledPin = 4;             // Pin number for LED

// Variables for timekeeping
int timer = 0;                    // Will count up to 10/40 seconds for acc. read
long startTime = 0;                // Will be set by micros() when start taking data
long reffTime = 0;                 // 
int datarate = 400;               // 
int sampleRate = 2;               // Rate (in milliseconds) data is taken from acc
int runTime = 5000;               // How long (in milliseconds) data is taken

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

void setup() {  
  Serial.begin(9600);    
  Serial.println("---------------------------------------");
  
  // Initialize the accelerometer (ADXL375)
  Serial.println("Initializing accelerometer (ADXL375)...");
  if(!accel.begin()) {    
    Serial.println("Accelerometer (ADXL375) failed.\n");
    while (1); 
  }
  // Data rate for accelerometer
  accel.setDataRate(ADXL343_DATARATE_400_HZ); 
  Serial.println("Initialization done.\n");

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

  // Button not needed in final design.
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);  
}

void loop() { 
  // Only try to read button input if not gathering data
  buttonState = digitalRead(buttonPin);  

  // Start reading from accelerometer
  if (buttonState) {    
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
    
    // **Probably need an if (fileName) statement to check if the file is active before reading/writing
    accFile = SD.open(fileName, FILE_WRITE);    

    if (accFile) {
      digitalWrite(ledPin, HIGH); // LED should be lit up while the SD card is taking data.
      accFile.println("time,xaxis,yaxis,zaxis"); // Adds first line for table headers    
      
      Serial.println(fileName); 
      Serial.println("Taking Data.");

      startTime = micros();

      // Runs while timer < runTime
      while (1) {      
        sensors_event_t event;  
        accel.getEvent(&event); 
        reffTime = micros();
        double currTime = (reffTime - startTime) / 10^6;

        // Variables for accelerometer axes
        double xData = event.acceleration.x;
        double yData = event.acceleration.y;
        double zData = event.acceleration.z;            
        
        // Comma/Line delimited for csv file
        String newLine = String(currTime) + "," + String(xData) + "," + String(yData) + "," + String(zData);      
        accFile.println(newLine);
        timer += sampleRate;        

        // Stop reading after runTime/1000 seconds
        if (timer == runTime) {
          timer = 0;   
          fileName = "";
          accFile.close();
          digitalWrite(ledPin, LOW);
          Serial.println("Done taking data.");    
          Serial.println("---------------------------------------");
          break;
        }
        delay(sampleRate);
      }
    } else {
        // There was an error accessing file for SD-card
        Serial.println("SD card not being written to.\n");
        while (1); // Program will stall here
      }  
  }  
}