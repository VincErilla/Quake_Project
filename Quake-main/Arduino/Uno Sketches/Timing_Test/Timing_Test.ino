#import <math.h>

long start, now;

void setup() {
  Serial.begin(115200);

  start = micros();
  Serial.println("Start Time: " + String(double(start) / pow(10.0, 6.0)));
  Serial.println("------------------");
}

void loop() {
  now = micros();
  Serial.print(now);
  Serial.print(" : ");
  Serial.println(double(now) / pow(10.0, 6.0), 4);
  delay(500);
}
