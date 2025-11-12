#include <Arduino.h>


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(2,OUTPUT);
}


void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  if (digitalRead(2)==HIGH) Serial.println("Button Pressed");
}