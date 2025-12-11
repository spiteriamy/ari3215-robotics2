#include "movement_set.h"
#include <Adafruit_NeoPixel.h>

#define PIN A2      // The image shows the green wire connected to Pin 7
#define NUMPIXELS 5 // Set this to the actual number of LEDs on your strip

// Setup the NeoPixel library
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint16_t cyclePos = 0;

// ultrasonic pins:
#define TRIG_PIN_1 13 // the pin to send out the ultrasonic signal
#define ECHO_PIN_1 12 // the pin to listen for the echo of the ultrasonic signal

#define TRIG_PIN_2 A0 // the pin to send out the ultrasonic signal
#define ECHO_PIN_2 A1 // the pin to listen for the echo of the ultrasonic signal

HCSR04 hc1(TRIG_PIN_1, ECHO_PIN_1); // ultrasonic sensor
HCSR04 hc2(TRIG_PIN_2, ECHO_PIN_2); // ultrasonic sensor 2

// Buzzer pin:
#define BUZZER_PIN 11

// servo pins and parameters:
#define SERVO_PIN 10   // the pin the servo is connected to
#define SERVO_STEP 10  // how many degrees to move the servo each time
#define SERVO_DELAY 20 // how many milliseconds to wait for the servo to reach the position
#define SERVO_MIN 0    // the minimum angle of the servo
#define SERVO_MID 90   // the middle angle of the servo
#define SERVO_MAX 180  // the maximum angle of the servo

Servo myservo; // servo object

int pos = SERVO_MID; // variable to store the servo position

// command and value from serial
int curr_cmd = -1, curr_val = -1, prev_cmd = -1, prev_val = -1;
int startTime = 0;
int duraThresh = -1; // milliseconds

// TODO: change to a5512a55b49b9d87ce6c20261df19dfda92ef45cdcd7398f0d70464f59b5055f2fec249a84b131207d5bc6dcff4ad5db7724e21928e548dfb4dd5664a4f6250c
int tweak = 1;

// flags
int line102 = 0, line126 = 0;

MovementSet move(100); // robot movement control

void obey(int left, int right)
{
  pixels.clear(); // Set all pixel colors to 'off'
  // float angles[] = {30.0f, 60.0f, 90.0f, 120.0f, 150.0f, 180.0f};
  switch (left)
  {
  case 0: // stop

    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // Turn all pixels red
    }
    pixels.show();

    move.stopMov();
    move.HALT = true;
    break;
  case 1: // forward

    for (int i = 0; i < right; i++)
    {
      pixels.setPixelColor(i, pixels.Color(255, 128, 0)); // Turn all pixels orange
    }

    move.uniformMov(1);
    break;
  case 2: // backward

    for (int i = 0; i < right; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // Turn all pixels blue
    }

    move.uniformMov(-1);
    break;
  case 3: // turn left
  {
    for (int i = 0; i < right; i++)
    {
      pixels.setPixelColor(i, pixels.Color(255, 255, 0)); // Turn all pixels green
    }

    float angle = (float)right;
    move.turn(angle);
    break;
  }
  case 4: // turn right
  {
    for (int i = 0; i < right; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // Turn all pixels green
    }

    float angle = (float)right;
    move.turn(angle);
    break;
  }
  case 5: // secret
    //  move.tweak();
    break;
  default:
    // do nothing
    break;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setup()
{
  Serial.begin(9600);

  // initialize the gyroscope
  move.initGyro();

  myservo.attach(SERVO_PIN); // attaches the servo on servo_pin to the servo object
  myservo.write(pos);        // tell servo to go to position in variable 'pos'

  pinMode(ECHO_PIN_1, INPUT);  // We receive input from the echo pin (World -> Arduino)
  pinMode(TRIG_PIN_1, OUTPUT); // We output to the trig pin (Arduino -> World)

  pinMode(BUZZER_PIN, OUTPUT); // buzzer pin

  move.setServo(myservo); // set the servo
  move.setHC(hc1);        // set the ultrasonic sensor

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.show();  // Turn OFF all pixels ASAP

  Serial.println("Arduino ready");
}

void loop()
{
  move.HALT = false;

  line102 = 0;
  line126 = 0;
  /*
    // avoid collision
    int dist = move.hc->dist();
    if (prev_cmd == 1 && dist < 10 && dist != 0){
        move.stopMov();
        move.HALT = true; // do we need this?
    }
*/
  if ((int)millis() - startTime >= duraThresh && duraThresh != -1)
  {
    // stop the robot
    move.stopMov();

    // reset values
    duraThresh = -1;
    prev_cmd = -1;
    prev_val = -1;

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
        startTime = (int)millis();    // get the start time
        duraThresh = curr_val * 1000; // milliseconds

        // check if time has run out
        if ((int)millis() - startTime < duraThresh)
        {
          Serial.print((int)millis() - startTime);
          Serial.print(" < ");
          Serial.print(duraThresh);
          Serial.print(" ");

          obey(curr_cmd, curr_val);
        }
      }
      else if (curr_cmd == 5)
      {
        if (curr_val == 5)
        {
          for (int i = 0; i < NUMPIXELS; i++)
          {
            pixels.setPixelColor(i, Wheel(cyclePos & 255));
          }
          pixels.show();
          cyclePos++;

          move.turn(tweak);
          tweak = -tweak;
        }
      }
      else
      {
        // Serial.print((float)curr_val);
        obey(curr_cmd, curr_val);
      }
    }

    // debug
    // Serial.print(data);
    Serial.print("Received: ");
    Serial.print(curr_cmd);
    Serial.print(" , ");
    Serial.print(curr_val);
    Serial.print(" ");
    /*Serial.print(move.HALT);
    Serial.print(" ");
    Serial.print(move.hc->dist());
    Serial.print(" ");
    Serial.print(millis() - startTime);
    Serial.print(" ");
    Serial.print(duraThresh);
    Serial.print(" ");
    Serial.print(prev_cmd);
    Serial.print(" ");
    Serial.print(prev_val);
    Serial.print(" ");
    Serial.print(line102 == 1);
    Serial.print(" ");*/
  }

  // EMERGENCY STOP IF OBSTACLE IS TOO CLOSE WHILE MOVING FORWARD OR BACKWARDS AND NOT ALREADY HALTED

  float fwd_dist = hc1.dist();
  float back_dist = hc2.dist();
  // if (dist < 10 && dist != 0) {
  //     move.stopMov();
  //     move.HALT = true;
  //     duraThresh = -1;
  //     Serial.print("EMERGENCY STOP ");
  // }
  Serial.println("dist=" + (String)fwd_dist + " ");

  // if were going forward, and the obstacle is too close in front
  if ((curr_cmd == 1 && fwd_dist < 30 && fwd_dist != 0) || (curr_cmd == 2 && back_dist < 30 && back_dist != 0))
  {
    // obstacle detected in front
    tone(BUZZER_PIN, 500); // Send 500Hz sound signal

    move.stopMov();
    move.HALT = true;
    duraThresh = -1;
    curr_cmd = -1;
  }
  else
  {
    // no obstacle detected in back
    noTone(BUZZER_PIN);

    // obstacleDetected = false;
    move.HALT = false;
  }

  // else
  // {
  //   if (curr_cmd == 1)
  //   {
  //     // Obstacle detected
  //     // flag to true

  //     fwd_obst = true;
  //     move.stopMov();
  //     move.HALT = true;
  //     duraThresh = -1;
  //     // Serial.print("dist="+(String)dist+" ");
  //   }
  // }

  // at the end of loop print newline to send this loop's telemetry to raspi
  // Serial.println("end");
}
