#include <Arduino.h>
#include <functional>

#define button1 4
#define trigger 8
#define echo 7
#define us_min_distance 10.00 // Minimum distance in cm to stop
#define mcc_fd 12             // forward direction
#define mcc_bd 13             // backward direction
#define mca 7                 // Relay
#define mcc_speed 11          // pwm pin

// for a constant speed
// #define speed 180 // Speed value from 0 to 255

#define pot A0 // used in this version
// sudo chmod a+rw /dev/ttyUSB0

// program functions
float measure_distance();
void mcc_control(int signal);
void blink_led(bool state);
void funct_every_ms(int interval_ms, std::function<void()> action);

void setup()
{
  pinMode(button1, INPUT);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trigger, LOW);
  pinMode(mcc_fd, OUTPUT);
  pinMode(mcc_bd, OUTPUT);
  pinMode(mcc_speed, OUTPUT);
  pinMode(pot, INPUT);
  // pinMode(mca,OUTPUT);     //install relay control
  mcc_control(0);
  Serial.begin(115200);
}

// FSM states
enum SystemState
{
  IDLE,
  FRONTWARD,
  BACKWARD,
  WAIT,
  END_COURSE
};
// program variables
SystemState currentState = IDLE;
long t_start = 0;
long t_travel = 0;
int speed = 0;
float distance = 0.000;
long t_lastloop = 0;

void loop()
{

  switch (currentState)
  {
  case IDLE:
    t_travel = 0; // Reset travel time at the beginning of IDLE
    if ((millis() - t_lastloop) >= 500) // Print every 500ms
    {
      t_lastloop = millis();
    Serial.println("System is idle.");
    }
    
    blink_led(true);
    if (digitalRead(button1) == HIGH)
    {
      // Antibounce: wait for button release
      while (digitalRead(button1) == HIGH)
      {
      }

      mcc_control(1);
      // digitalWrite(mca,HIGH);

      currentState = FRONTWARD;
      t_start = millis(); // Start time when entering cases
    }
    break;

  case FRONTWARD:
    mcc_control(1);
    blink_led(false);
    if ((millis() - t_lastloop) >= 100) // measure every 100ms
    {
      t_lastloop = millis();
      distance = measure_distance();
      
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" cm     Going frontward");
    }
    if (digitalRead(button1) == HIGH)
    {
      // Antibounce: wait for button release
      while (digitalRead(button1) == HIGH)
      {
      }

      t_travel = (millis() - t_start); // save travel time until now(5s)

      currentState = WAIT;
      t_start = millis(); // t_start is the instant it enters in waiting (50+5)
    }
    else if (measure_distance() < us_min_distance) // or button press
    {

      currentState = END_COURSE;
      t_travel = millis() - t_start;

      // digitalWrite(mca,LOW);
    }

    break;
  case BACKWARD:

    // Currently not used
    break;

  case WAIT:
    mcc_control(0);
    blink_led(true);
    print_every_ms(500, "Waiting...");

    // digitalWrite(mca_speed, 0);
    if (digitalRead(button1) == HIGH)
    {
      long t_keepbutton = millis();
      // Antibounce: wait for button release
      while (digitalRead(button1) == HIGH)
      {
        if (millis() - t_keepbutton > 2000) // long press detected go backwards until realease
        {
          mcc_control(-1);
          distance = measure_distance();
          blink_led(false);
          Serial.print("Distance: ");
          Serial.print(distance);
          Serial.println(" cm     Going backward");
          t_start = millis();
        }
      }
      if (millis() - t_keepbutton >= 2000)
      {
        break;
      }
      // implement logic for resuming frontward or backward
      currentState = FRONTWARD;
      t_start = millis(); // Reset start time when resuming
    }
    break;

  case END_COURSE:
    print_every_ms(500, "End of course. Time:"+String(t_travel / 1000.00)+" s" );
    if (digitalRead(button1) == HIGH)
    {
      // Antibounce: wait for button release
      while (digitalRead(button1) == HIGH)
      {
      }
      mcc_control(-1);
      delay(t_travel * 0.9); // implement sensor reading until IDLE
      mcc_control(0);
      currentState = IDLE;
    }
    break;
  }
}

float measure_distance()
{
  float duration;
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  duration = pulseIn(echo, HIGH) / 58;

  return duration;
}

void mcc_control(int signal)
{
  // Update speed from potentiometer only when explicitly requested.

    Serial.print("Updating speed from potentiometer: ");
    speed = map(analogRead(pot), 0, 1023, 0, 255);
    Serial.println(analogRead(pot));
    Serial.print("Mapped speed: ");
    Serial.println(speed);
  

  // Recieve command and controls mcc_bd, mcc_fd and mcc_speed
  if (signal == 1)
  {
    digitalWrite(mcc_fd, HIGH);
    digitalWrite(mcc_bd, LOW);
    analogWrite(mcc_speed, speed);
  }
  else if (signal == -1)
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
  funct_every_ms(50, [state](){ digitalWrite(LED_BUILTIN, state ? HIGH : LOW); }); // Turn the LED on or off
  funct_every_ms(50, [](){ digitalWrite(LED_BUILTIN, LOW); });
}

// Call an action every `interval_ms`. This does NOT print or measure by itself.
// action: callable (e.g., lambda) to be called when the interval elapses.
// this function should be on a loop to well function
void funct_every_ms(int interval_ms, std::function<void()> action) {
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= (unsigned long)interval_ms) {
    lastTime = millis();
    action();
  }
}

void print_every_ms(int interval_ms, String message) {
  
  funct_every_ms(interval_ms, [message](){ Serial.println(message); });
  
}



