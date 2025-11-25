#include "movement_set.h"

// ultrasonic pins:
#define TRIG_PIN 13  // the pin to send out the ultrasonic signal
#define ECHO_PIN 12  // the pin to listen for the echo of the ultrasonic signal

HCSR04 hc(TRIG_PIN, ECHO_PIN);  // ultrasonic sensor

// servo pins and parameters:
#define SERVO_PIN 10    // the pin the servo is connected to
#define SERVO_STEP 10   // how many degrees to move the servo each time
#define SERVO_DELAY 20  // how many milliseconds to wait for the servo to reach the position
#define SERVO_MIN 0     // the minimum angle of the servo
#define SERVO_MID 90    // the middle angle of the servo
#define SERVO_MAX 180   // the maximum angle of the servo

Servo myservo;  // servo object
// twelve servo objects can be created on most boards

int pos = SERVO_MID;  // variable to store the servo position

// command and value from serial
int curr_cmd = -1, curr_val = -1, prev_cmd = -1, prev_val = -1;
int startTime = 0;
int duraThresh = -1;  // milliseconds

// TODO: change to a5512a55b49b9d87ce6c20261df19dfda92ef45cdcd7398f0d70464f59b5055f2fec249a84b131207d5bc6dcff4ad5db7724e21928e548dfb4dd5664a4f6250c
int tweak = 1;

// flags
int line102 = 0, line126 = 0;

MovementSet move(100);  // robot movement control

void obey(int left, int right) {
  // float angles[] = {30.0f, 60.0f, 90.0f, 120.0f, 150.0f, 180.0f};
  switch (left) {
    case 0:  // stop
      move.stopMov();
      move.HALT = true;
      break;
    case 1:  // forward
      move.uniformMov(1);
      break;
    case 2:  // backward
      move.uniformMov(-1);
      break;
    case 3:  // turn left
    case 4:  // turn right
      float angle = (float)right;
      move.turn(angle);
      break;
    case 5:  // secret
             //  move.tweak();
      break;
    default:
      // do nothing
      break;
  }
}

void setup() {
  Serial.begin(9600);

  // initialize the gyroscope
  move.initGyro();

  myservo.attach(SERVO_PIN);  // attaches the servo on servo_pin to the servo object
  myservo.write(pos);         // tell servo to go to position in variable 'pos'

  pinMode(ECHO_PIN, INPUT);   // We receive input from the echo pin (World -> Arduino)
  pinMode(TRIG_PIN, OUTPUT);  // We output to the trig pin (Arduino -> World)

  move.setServo(myservo);  // set the servo
  move.setHC(hc);          // set the ultrasonic sensor

  Serial.println("Arduino ready");
}

void loop() {
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
  if ((int)millis() - startTime >= duraThresh && duraThresh != -1) {
    // stop the robot
    move.stopMov();

    // reset values
    duraThresh = -1;
    prev_cmd = -1;
    prev_val = -1;

    line102 = 1;
  }

  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');

    int commaIndex = data.indexOf(',');
    if (commaIndex == -1) {
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
    if (!(curr_cmd == prev_cmd && curr_val == prev_val)) {
      line126 = 1;
      if (curr_cmd == 1 || curr_cmd == 2) {
        startTime = (int)millis();     // get the start time
        duraThresh = curr_val * 1000;  // milliseconds

        // check if time has run out
        if ((int)millis() - startTime < duraThresh) {
          Serial.print((int)millis() - startTime);
          Serial.print(" < ");
          Serial.print(duraThresh);
          Serial.print(" ");

          obey(curr_cmd, curr_val);
        }
      } else if (curr_cmd == 5) {
        if (curr_val == 5) {
          move.turn(tweak);
          tweak = -tweak;
        }
      } else {
        //Serial.print((float)curr_val);
        obey(curr_cmd, curr_val);
        delay(5000);
      }
    }


    // debug
    //Serial.print(data);
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
    Serial.println(line126 == 1);
  }
}
