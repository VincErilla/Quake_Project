#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL375.h>
#include <math.h>

Adafruit_ADXL375 acc1 = Adafruit_ADXL375(1);
Adafruit_ADXL375 acc2 = Adafruit_ADXL375(2);

unsigned long startTime, nowTime;
double runTime = 5.0, currTime = 0.0;

void setup() {
  Serial.begin(9600);  
  
  // Initialize the first accelerometer (ADXL375)
  Serial.println("Initializing accelerometer #1 (ADXL375)...");
  if(!acc1.begin()) {    
    Serial.println("Accelerometer #1 (ADXL375) failed to initialize.\n");
    while (1); 
  }
  acc1.setDataRate(ADXL343_DATARATE_100_HZ); 
  Serial.println("Initialization done.\n");

  // Initialize the second accelerometer (ADXL375)
  Serial.println("Initializing accelerometer #2 (ADXL375)...");
  if(!acc2.begin()) {    
    Serial.println("Accelerometer #2 (ADXL375) failed to initialize.\n");
    while (1); 
  }
  acc2.setDataRate(ADXL343_DATARATE_100_HZ); 
  Serial.println("Initialization done.\n");

  Serial.println("-------------------------------------");
}

void loop() {  
  startTime = micros();
  currTime = 0.0;

  while (currTime < runTime) {
    sensors_event_t a1, a2;  
    acc1.getEvent(&a1);
    acc2.getEvent(&a2); 

    nowTime = micros();
    double xData1 = a1.acceleration.x;
    double yData1 = a1.acceleration.y;
    double zData1 = a1.acceleration.z; 
    double xData2 = a2.acceleration.x;
    double yData2 = a2.acceleration.y;
    double zData2 = a2.acceleration.z; 

    currTime = double(nowTime - startTime) / pow(10.0, 6.0);
    Serial.print(currTime);
    Serial.print(": ");
    Serial.print(xData1);
    Serial.print(", ");
    Serial.print(yData1);
    Serial.print(", ");
    Serial.print(zData1);  
    Serial.print(" | "); 
    Serial.print(xData2);
    Serial.print(", ");
    Serial.print(yData2);
    Serial.print(", ");
    Serial.println(zData2);     
  }  
  while(1);
}
