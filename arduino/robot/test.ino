#include "movement_set.h"
#include <Adafruit_NeoPixel.h>
#include <Servo.h>

// ---------------- LED STRIP ----------------
#define PIN A2      // change this if your data wire is on a different pin
#define NUMPIXELS 5 // number of LEDs on your strip

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setAllPixels(uint32_t color)
{
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

// ---------------- ULTRASONIC ----------------
#define TRIG_PIN_1 13
#define ECHO_PIN_1 12

#define TRIG_PIN_2 A0
#define ECHO_PIN_2 A1

HCSR04 hc1(TRIG_PIN_1, ECHO_PIN_1);
HCSR04 hc2(TRIG_PIN_2, ECHO_PIN_2);

// ---------------- BUZZER ----------------
#define BUZZER_PIN 11

// ---------------- SERVO ----------------
#define SERVO_PIN 10
#define SERVO_STEP 10
#define SERVO_DELAY 20
#define SERVO_MIN 0
#define SERVO_MID 90
#define SERVO_MAX 180

Servo myservo;
int pos = SERVO_MID;

// ---------------- SERIAL COMMANDS ----------------
int curr_cmd = -1, curr_val = -1, prev_cmd = -1, prev_val = -1;
int startTime = 0;
int duraThresh = -1; // milliseconds

int tweak = 1;

// flags
int line102 = 0, line126 = 0;

MovementSet move(100); // robot movement control

// ---------------- COMMAND EXECUTOR ----------------
void obey(int left, int right)
{
  switch (left)
  {
  case 0: // Halt()
    setAllPixels(pixels.Color(255, 0, 0)); // red
    move.stopMov();
    move.HALT = true;
    break;

  case 1: // Move Forward()
    setAllPixels(pixels.Color(255, 192, 203)); // whitish pink
    move.uniformMov(1);
    break;

  case 2: // Move Backward()
    setAllPixels(pixels.Color(0, 0, 255)); // blue
    move.uniformMov(-1);
    break;

  case 3: // Turn Left()
    setAllPixels(pixels.Color(255, 255, 0)); // yellow
    move.turn((float)right);
    break;

  case 4: // Turn Right()
    setAllPixels(pixels.Color(0, 255, 0)); // green
    move.turn((float)right);
    break;

  case 5: // Secret()
    setAllPixels(pixels.Color(128, 0, 128)); // purple
    break;

  default:
    break;
  }
}

void setup()
{
  Serial.begin(9600);

  // initialize the gyroscope
  move.initGyro();

  myservo.attach(SERVO_PIN);
  myservo.write(pos);

  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_1, OUTPUT);

  pinMode(ECHO_PIN_2, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  move.setServo(myservo);
  move.setHC(hc1);

  pixels.begin();
  setAllPixels(pixels.Color(0, 0, 0)); // off

  Serial.println("Arduino ready");
}

void loop()
{
  move.HALT = false;

  line102 = 0;
  line126 = 0;

  // stop after duration ends (for forward/back commands that use time)
  if ((int)millis() - startTime >= duraThresh && duraThresh != -1)
  {
    obey(0, 0); // Halt() -> red LEDs + stop

    // reset values
    duraThresh = -1;
    prev_cmd = -1;
    prev_val = -1;
    curr_cmd = -1;

    line102 = 1;
  }

  if (Serial.available() > 0)
  {
    String data = Serial.readStringUntil('\n');

    int commaIndex = data.indexOf(',');
    if (commaIndex == -1)
    {
      return;
    }

    // save the previous command and value
    prev_cmd = curr_cmd;
    prev_val = curr_val;

    // parse the current command and value
    curr_cmd = data.substring(0, commaIndex).toInt();
    curr_val = data.substring(commaIndex + 1, data.length()).toInt();

    Serial.print("recieved val:");
    Serial.print(curr_val);
    Serial.print(" ");

    // send commands to obey if they are different from the previous ones
    if (!(curr_cmd == prev_cmd && curr_val == prev_val))
    {
      line126 = 1;

      if (curr_cmd == 1 || curr_cmd == 2)
      {
        startTime = (int)millis();
        duraThresh = curr_val * 1000;

        if ((int)millis() - startTime < duraThresh)
        {
          Serial.print((int)millis() - startTime);
          Serial.print(" < ");
          Serial.print(duraThresh);
          Serial.print(" ");

          obey(curr_cmd, curr_val); // LED color + movement
        }
      }
      else if (curr_cmd == 5)
      {
        obey(5, curr_val); // purple LEDs

        // keep your secret movement (optional)
        if (curr_val == 5)
        {
          move.turn(tweak);
          tweak = -tweak;
        }
      }
      else
      {
        obey(curr_cmd, curr_val);
      }
    }

    Serial.print("Received: ");
    Serial.print(curr_cmd);
    Serial.print(" , ");
    Serial.print(curr_val);
    Serial.print(" ");
  }

  // EMERGENCY STOP IF OBSTACLE IS TOO CLOSE WHILE MOVING FORWARD OR BACKWARDS
  float fwd_dist = hc1.dist();
  float back_dist = hc2.dist();

  Serial.println("dist=" + (String)fwd_dist + " ");

  if ((curr_cmd == 1 && fwd_dist < 30 && fwd_dist != 0) ||
      (curr_cmd == 2 && back_dist < 30 && back_dist != 0))
  {
    tone(BUZZER_PIN, 500);

    obey(0, 0); // Halt() -> red LEDs + stop
    duraThresh = -1;
    curr_cmd = -1;
  }
  else
  {
    noTone(BUZZER_PIN);
    move.HALT = false;
  }
}
