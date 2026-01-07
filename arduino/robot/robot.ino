#include "movement_set.h"
#include <Adafruit_NeoPixel.h>
#include "pitches.h"

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

int melody[] = {
  NOTE_E5, NOTE_D5, NOTE_FS4, NOTE_GS4, 
  NOTE_CS5, NOTE_B4, NOTE_D4, NOTE_E4, 
  NOTE_B4, NOTE_A4, NOTE_CS4, NOTE_E4,
  NOTE_A4
};

int durations[] = {
  8, 8, 4, 4,
  8, 8, 4, 4,
  8, 8, 4, 4,
  2
};

// servo pins and parameters:
#define SERVO_PIN 10   // the pin the servo is connected to
#define SERVO_STEP 10  // how many degrees to move the servo each time
#define SERVO_DELAY 20 // how many milliseconds to wait for the servo to reach the position
#define SERVO_MIN 0    // the minimum angle of the servo
#define SERVO_MID 90   // the middle angle of the servo
#define SERVO_MAX 180  // the maximum angle of the servo

// Servo Angles
#define SERVO_LEFT 170
#define SERVO_CENTER 96
#define SERVO_RIGHT 10

Servo myservo; // servo object

int pos = SERVO_CENTER; // variable to store the servo position

// command and value from serial
int curr_cmd = -1, curr_val = -1, prev_cmd = -1, prev_val = -1;
int startTime = 0;
int duraThresh = -1; // milliseconds

MovementSet move(100); // robot movement control


// ---------------- LED HELPERS ----------------
// Turn ON only the first N pixels with a given color (rest OFF)
void setPixelsCount(uint32_t color, int count)
{
  pixels.clear();

  // clamp count to valid range
  if (count < 0)
    count = 0;
  if (count > NUMPIXELS)
    count = NUMPIXELS;

  for (int i = 0; i < count; i++)
  {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

// If your right value is an ANGLE for turning (30..180),
// convert that to "finger count" (1..5) just for LEDs.
int angleToLedCount(int angle)
{
  if (abs(angle) == 0)
    return 0;
  if (abs(angle) == 30)
    return 1;
  if (abs(angle) == 60)
    return 2;
  if (abs(angle) == 90)
    return 3;
  if (abs(angle) == 120)
    return 4;
  return 5; // 150..180 etc
}

// Secret flash mode
bool secretMode = false;
unsigned long lastFlashMs = 0;
int flashIndex = 0;
const unsigned long FLASH_INTERVAL_MS = 75; // bit fast

const int SECRET_COLOR_COUNT = 6;
uint32_t secretColors[SECRET_COLOR_COUNT];

void initSecretColors()
{
  secretColors[0] = pixels.Color(255, 0, 0);     // red
  secretColors[1] = pixels.Color(128, 0, 128);   // purple
  secretColors[2] = pixels.Color(0, 0, 255);     // blue
  secretColors[3] = pixels.Color(255, 255, 0);   // yellow
  secretColors[4] = pixels.Color(0, 255, 0);     // green
  secretColors[5] = pixels.Color(255, 192, 203); // pink
}

void startSecretFlash()
{
  secretMode = true;
  flashIndex = 0;
  lastFlashMs = 0;
}

void stopSecretFlash()
{
  secretMode = false;
}

void updateSecretFlash()
{
  unsigned long now = millis();
  if (now - lastFlashMs >= FLASH_INTERVAL_MS)
  {
    lastFlashMs = now;
    setPixelsCount(secretColors[flashIndex], NUMPIXELS);
    flashIndex = (flashIndex + 1) % SECRET_COLOR_COUNT;
  }
}
// --------------------------------------------




float getFastDist(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  // 20000 microseconds = ~3.4 meters. 
  // If no object is hit, it returns 0 after only 20ms instead of 1000ms.
  long duration = pulseIn(echo, HIGH, 20000); 
  if (duration == 0) return 400.0; // No obstacle found
  return duration * 0.034 / 2;
}



void obey(int left, int right)
{
  pixels.clear(); // Set all pixel colors to 'off'
  // float angles[] = {30.0f, 60.0f, 90.0f, 120.0f, 150.0f, 180.0f};
  switch (left)
  {
  case 0: // stop

    stopSecretFlash();
    setPixelsCount(pixels.Color(255, 0, 0), NUMPIXELS); // red

    move.stopMov();
    move.HALT = true;
    break;
  case 1: // forward

    stopSecretFlash();
    setPixelsCount(pixels.Color(128, 0, 128), right); // purple

    move.uniformMov(1);
    break;
  case 2: // backward

    stopSecretFlash();
    setPixelsCount(pixels.Color(0, 0, 255), right); // blue

    move.uniformMov(-1);
    break;
  case 3: // turn left
  {
    stopSecretFlash();
    setPixelsCount(pixels.Color(255, 255, 0), angleToLedCount(right)); // yellow

    delay(500);
    float angle = (float)right;
  
    bool success = move.turn(angle);
    if (!success) {
      setPixelsCount(pixels.Color(255, 255, 255), NUMPIXELS); // white
    }

    break;
  }
  case 4: // turn right
  {
    stopSecretFlash();
    setPixelsCount(pixels.Color(0, 255, 0), angleToLedCount(right)); // green

    delay(500);
    float angle = (float)right;
    bool success = move.turn(angle);
    if (!success)
    {
      setPixelsCount(pixels.Color(255, 255, 255), NUMPIXELS); // white
    }

    break;
  }
  case 5: // secret
    //  move.tweak();
    startSecretFlash();
    updateSecretFlash();
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

void playNokiaRingtone() {
  int size = sizeof(durations) / sizeof(durations[0]);

  for (int note = 0; note < size; note++) {
    int duration = 1000 / durations[note];

    tone(BUZZER_PIN, melody[note], duration);

    // duration + 30% gap
    int pauseBetweenNotes = (duration * 13) / 10;
    delay(pauseBetweenNotes);

    noTone(BUZZER_PIN);
  }
}


// Smooth Servo Movement
void slowServoMove(int fromAngle, int toAngle) {
  int step = (fromAngle < toAngle) ? 1 : -1;
  for (int pos = fromAngle; pos != toAngle; pos += step) {
    myservo.write(pos);
    delay(10); // slows down movement
  }
  myservo.write(toAngle); // ensure it lands on final angle
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

  // move.setServo(myservo); // set the servo
  move.setHC(hc1);        // set the ultrasonic sensor

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.show();  // Turn OFF all pixels ASAP

  initSecretColors();
  setPixelsCount(pixels.Color(0, 0, 0), NUMPIXELS); // all off

  Serial.println("Arduino ready");
}

void loop()
{
  move.HALT = false;

  // keep flashing while secretMode is active
  if (secretMode)
  {
    updateSecretFlash();
  }

  if ((int)millis() - startTime >= duraThresh && duraThresh != -1)
  {
    // stop the robot
    move.stopMov();

    // LED-only: stop flashing + show red
    stopSecretFlash();
    setPixelsCount(pixels.Color(255, 0, 0), NUMPIXELS);

    // reset values
    duraThresh = -1;
    prev_cmd = -1;
    prev_val = -1;
  }

  if (Serial.available() > 0)
  {
    String data = Serial.readStringUntil(';');
    Serial.print("data: ");
    Serial.print(data);

    int commaIndex = data.indexOf(',');
    // int semiIndex = data.indexOf(';');
    if (commaIndex == -1)
    {
      return;
    }

    // save the previous command and value
    prev_cmd = curr_cmd;
    prev_val = curr_val;


    // parse the current command and value
    curr_cmd = data.substring(0, commaIndex).toInt();
    curr_val = data.substring(commaIndex + 1,data.length()).toInt();

    // send commands to obey if they are different from the previous ones
    if (!(curr_cmd == prev_cmd && curr_val == prev_val))
    {
      if (curr_cmd == 1 || curr_cmd == 2)
      {
        startTime = (int)millis();    // get the start time
        duraThresh = curr_val * 1000; // milliseconds

        // check if time has run out
        if ((int)millis() - startTime < duraThresh)
        {

          obey(curr_cmd, curr_val);
        }
      }
      else if (curr_cmd == 5)
      {
        if (curr_val == 5)
        {
          noTone(BUZZER_PIN);   // stop any obstacle beep
          move.stopMov();       // don't drive while we're delaying
          move.HALT = true;

          playNokiaRingtone();

          move.HALT = false;
          obey(5, curr_val);

          move.turn(rand() % 2 == 0 ? -1 : 1);

          cyclePos++;
        }
      }

      else if(curr_cmd == 3 || curr_cmd == 4) {
        float dist;

        if(curr_cmd == 3) { // leeft
          slowServoMove(SERVO_CENTER, SERVO_LEFT);
          delay(300);
          dist = getFastDist(TRIG_PIN_1, ECHO_PIN_1);
          slowServoMove(SERVO_LEFT, SERVO_CENTER);
          delay(300);
        }
        else if(curr_cmd == 4) { // right
          slowServoMove(SERVO_CENTER, SERVO_RIGHT);
          delay(300);
          dist = getFastDist(TRIG_PIN_1, ECHO_PIN_1);
          slowServoMove(SERVO_RIGHT, SERVO_CENTER);
          delay(300);
        }
        
        if (dist < 30 && dist != 0) {
          // obstacle detected
          tone(BUZZER_PIN, 500); // Send 500Hz sound signal

          move.stopMov();
          move.HALT = true;

          curr_cmd = -1;
        }
        else
        {
          // no obstacle detected
          noTone(BUZZER_PIN);

          move.HALT = false;
          obey(curr_cmd, (float)curr_val);
        }

        // turn servo back to center
        // if (curr_cmd == 4) {
        //   slowServoMove(SERVO_RIGHT, SERVO_CENTER);
        //   delay(300);
        // }
        // else if(curr_cmd == 3){
        //   slowServoMove(SERVO_LEFT, SERVO_CENTER);
        //   delay(300);
        // }
        
      }

      else
      {
        // Serial.print((float)curr_val);
        obey(curr_cmd, curr_val);
      }
    }


  }

  // EMERGENCY STOP IF OBSTACLE IS TOO CLOSE WHILE MOVING FORWARD OR BACKWARDS AND NOT ALREADY HALTED

  float fwd_dist = getFastDist(TRIG_PIN_1, ECHO_PIN_1);
  float back_dist = getFastDist(TRIG_PIN_2, ECHO_PIN_2);


  // if were going forward, and the obstacle is too close in front
  if ((curr_cmd == 1 && fwd_dist < 30 && fwd_dist != 0) || (curr_cmd == 2 && back_dist < 30 && back_dist != 0))
  {
    // obstacle detected in front
    tone(BUZZER_PIN, 500); // Send 500Hz sound signal

    move.stopMov();
    move.HALT = true;

    // LED-only: stop flashing + show red
    stopSecretFlash();
    setPixelsCount(pixels.Color(255, 0, 0), NUMPIXELS);

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


}