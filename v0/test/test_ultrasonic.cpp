#include <Arduino.h>
#define button1 4
#define trigger 7
#define echo 8
#define u_sonic_sp 50
#define mcc 5
#define mca 6 

float measure_distance();
// sudo chmod a+rw /dev/ttyUSB0
void setup() {
  // put your setup code here, to run once:

  pinMode(button1,INPUT);
  pinMode(trigger,OUTPUT);
  pinMode(echo,INPUT);
  digitalWrite(trigger,LOW);
  // digitalWrite(LED_BUILTIN,LOW);
  //pinmode(mcc,OUTPUT);
  //pinmode(mca,OUTPUT);
  //digitalWrite(mcc,LOW);
  
  Serial.begin(115200);
}

void loop() {
  delay(5000);
  // put your main code here, to run repeatedly:
  Serial.print("Distance: ");
  Serial.print(measure_distance());
  Serial.println(" cm");
  delay(100);
}


float measure_distance()
{
  float duration;
  digitalWrite(trigger,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger,LOW);
  duration=pulseIn(echo,HIGH)/58;
  
  return duration;
}
