#include "movement_set.h"
#include <Adafruit_NeoPixel.h>

#define PIN A2
#define NUMPIXELS 5

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint16_t cyclePos = 0;

// ultrasonic pins:
#define TRIG_PIN_1 13
#define ECHO_PIN_1 12

#define TRIG_PIN_2 A0
#define ECHO_PIN_2 A1

HCSR04 hc1(TRIG_PIN_1, ECHO_PIN_1);
HCSR04 hc2(TRIG_PIN_2, ECHO_PIN_2);

// Buzzer pin:
#define BUZZER_PIN 11

// servo pins and parameters:
#define SERVO_PIN 10
#define SERVO_STEP 10
#define SERVO_DELAY 20
#define SERVO_MIN 0
#define SERVO_MID 90
#define SERVO_MAX 180

Servo myservo;
int pos = SERVO_MID;

// command and value from serial
int curr_cmd = -1, curr_val = -1, prev_cmd = -1, prev_val = -1;
int startTime = 0;
int duraThresh = -1; // milliseconds

int tweak = 1;

// flags
int line102 = 0, line126 = 0;

MovementSet move(100);

// ---------------- LED HELPERS ----------------
void setAllPixels(uint32_t color)
{
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, color);
  }
  pixels.show();
}

// Secret flash mode (bit fast)
bool secretMode = false;
unsigned long lastFlashMs = 0;
int flashIndex = 0;
const unsigned long FLASH_INTERVAL_MS = 75; // bit fast

const int SECRET_COLOR_COUNT = 6;
uint32_t secretColors[SECRET_COLOR_COUNT];

void initSecretColors()
{
  // your “registered” colours
  secretColors[0] = pixels.Color(255, 0, 0);     // red (halt)
  secretColors[1] = pixels.Color(128, 0, 128);   // purple (forward)
  secretColors[2] = pixels.Color(0, 0, 255);     // blue (backward)
  secretColors[3] = pixels.Color(255, 255, 0);   // yellow (left)
  secretColors[4] = pixels.Color(0, 255, 0);     // green (right)
  secretColors[5] = pixels.Color(255, 192, 203); // pink (secret)
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
    setAllPixels(secretColors[flashIndex]);
    flashIndex = (flashIndex + 1) % SECRET_COLOR_COUNT;
  }
}
// --------------------------------------------

void obey(int left, int right)
{
  switch (left)
  {
  case 0: // stop
    stopSecretFlash();
    setAllPixels(pixels.Color(255, 0, 0)); // red
    move.stopMov();
    move.HALT = true;
    break;

  case 1: // forward
    stopSecretFlash();
    setAllPixels(pixels.Color(128, 0, 128)); // purple
    move.uniformMov(1);
    break;

  case 2: // backward
    stopSecretFlash();
    setAllPixels(pixels.Color(0, 0, 255)); // blue
    move.uniformMov(-1);
    break;

  case 3: // turn left
  {
    stopSecretFlash();
    setAllPixels(pixels.Color(255, 255, 0)); // yellow
    float angle = (float)right;
    move.turn(angle);
    break;
  }

  case 4: // turn right
  {
    stopSecretFlash();
    setAllPixels(pixels.Color(0, 255, 0)); // green
    float angle = (float)right;
    move.turn(angle);
    break;
  }

  case 5: // secret -> START FAST FLASH HERE (as you asked)
    startSecretFlash();
    updateSecretFlash(); // immediate first flash (no waiting)
    break;

  default:
    break;
  }
}

// kept (even if not used now)
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
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

  move.initGyro();

  myservo.attach(SERVO_PIN);
  myservo.write(pos);

  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_1, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  move.setServo(myservo);
  move.setHC(hc1);

  pixels.begin();
  pixels.show();

  initSecretColors();
  setAllPixels(pixels.Color(0, 0, 0));

  Serial.println("Arduino ready");
}

void loop()
{
  move.HALT = false;

  line102 = 0;
  line126 = 0;

  // keep flashing while secretMode is active
  if (secretMode)
  {
    updateSecretFlash();
  }

  if ((int)millis() - startTime >= duraThresh && duraThresh != -1)
  {
    move.stopMov();

    // LED-only: stop flashing + show red
    stopSecretFlash();
    setAllPixels(pixels.Color(255, 0, 0));

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

    prev_cmd = curr_cmd;
    prev_val = curr_val;

    curr_cmd = data.substring(0, commaIndex).toInt();
    curr_val = data.substring(commaIndex + 1, data.length()).toInt();

    Serial.print("recieved val:");
    Serial.print(curr_val);
    Serial.print(" ");

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

          obey(curr_cmd, curr_val);
        }
      }
      else if (curr_cmd == 5)
      {
        if (curr_val == 5)
        {
          // start flashing via case 5
          obey(5, curr_val);

          // keep your original secret movement logic
          move.turn(tweak);
          tweak = -tweak;

          cyclePos++;
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

  float fwd_dist = hc1.dist();
  float back_dist = hc2.dist();

  Serial.println("dist=" + (String)fwd_dist + " ");

  if ((curr_cmd == 1 && fwd_dist < 30 && fwd_dist != 0) || (curr_cmd == 2 && back_dist < 30 && back_dist != 0))
  {
    tone(BUZZER_PIN, 500);

    move.stopMov();
    move.HALT = true;

    // LED-only: stop flashing + show red
    stopSecretFlash();
    setAllPixels(pixels.Color(255, 0, 0));

    duraThresh = -1;
    curr_cmd = -1;
  }
  else
  {
    noTone(BUZZER_PIN);
    move.HALT = false;
  }
}
