int latchPin = 7;
int ledPin = 4;

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(latchPin, HIGH);
}

void loop() {
  digitalWrite(ledPin, HIGH);
  delay(5000);
  digitalWrite(latchPin, LOW);
}
