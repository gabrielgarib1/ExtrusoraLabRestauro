#include <Arduino.h>
#define button1 4
#define trigger 8
#define echo 7
#define us_min_distance 10.00 // Minimum distance in cm to stop
#define mcc_fd 12  // forward direction
#define mcc_bd 13  // backward direction
#define mca 7 // Relay
#define mcc_speed 11 //pwm pin

//for a constant speed
//#define speed 180 // Speed value from 0 to 255 

#define pot A0 // used in this version
// sudo chmod a+rw /dev/ttyUSB0

//program functions
float measure_distance();
void mcc_control(int signal = 0, bool updateSpeed = false);
void blink_led(bool state);

void setup() {
  pinMode(button1,INPUT);
  pinMode(trigger,OUTPUT);
  pinMode(echo,INPUT);
  digitalWrite(trigger,LOW);
  pinMode(mcc_fd,OUTPUT);
  pinMode(mcc_bd,OUTPUT);
  pinMode(mcc_speed,OUTPUT);
  pinMode(pot,INPUT);
  //pinMode(mca,OUTPUT);
  mcc_control(0);
  Serial.begin(115200);
}

// FSM states
enum SystemState {
  IDLE,
  MEASURING_DISTANCE,
  END_COURSE
};
// program variables
SystemState currentState = IDLE;
long loop_time = 0;
long travel_time = 0;
int speed = 0;

void loop() {
  
  switch(currentState) {
    case IDLE:
      Serial.println("System is idle.");
      blink_led(true);
      if(digitalRead(button1) == HIGH) {
        // Antibounce: wait for button release
        while(digitalRead(button1) == HIGH) {
        }

  currentState = MEASURING_DISTANCE;
  loop_time = millis();
  // Update speed from potentiometer when starting movement
  mcc_control(1, true);
        // digitalWrite(mca,HIGH);
      } 
      break;
      
    case MEASURING_DISTANCE:
      blink_led(false);
      Serial.println("Measuring distance...");
      delay(1000);
      Serial.print("Distance: ");
      Serial.print(measure_distance());
      Serial.println(" cm");
      if(measure_distance() < us_min_distance) {
        currentState = END_COURSE;
        travel_time = (millis() - loop_time);
        // digitalWrite(mca,LOW);
      } 
      break;
    
    case END_COURSE:
      Serial.println("End of course. Time: " + String(travel_time/1000.00) + " s");
      mcc_control(-1);
      delay(travel_time);
      mcc_control(0);
      currentState = IDLE;
      break;
  }
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

void mcc_control(int signal, bool updateSpeed)
{
  // Update speed from potentiometer only when explicitly requested.
  if (updateSpeed) {
    Serial.print("Updating speed from potentiometer: ");
    speed = map(analogRead(pot), 0, 1023, 0, 255);
    Serial.println(analogRead(pot));
    Serial.print("Mapped speed: ");
    Serial.println(speed);
  }

  // Recieve command and controls mcc_bd, mcc_fd and mcc_speed
  if (signal == 1) {
    digitalWrite(mcc_fd, HIGH);
    digitalWrite(mcc_bd, LOW);
    analogWrite(mcc_speed, speed);
  }
  else if (signal==-1)
  {
    digitalWrite(mcc_fd, LOW);
    digitalWrite(mcc_bd, HIGH);
    analogWrite(mcc_speed, speed);
  }
  else if (signal == 0)
  {
    digitalWrite(mcc_fd, LOW);
    digitalWrite(mcc_bd, LOW);
    analogWrite(mcc_speed, 0);
  }
}

void blink_led(bool state)
{
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW); // Turn the LED on or off
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}