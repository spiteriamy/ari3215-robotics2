#include "movement_set.h"

// ultrasonic pins:
#define TRIG_PIN 13 // the pin to send out the ultrasonic signal
#define ECHO_PIN 12 // the pin to listen for the echo of the ultrasonic signal

HCSR04 hc(TRIG_PIN, ECHO_PIN); // ultrasonic sensor

// servo pins and parameters:
#define SERVO_PIN 10   // the pin the servo is connected to
#define SERVO_STEP 10  // how many degrees to move the servo each time
#define SERVO_DELAY 20 // how many milliseconds to wait for the servo to reach the position
#define SERVO_MIN 0    // the minimum angle of the servo
#define SERVO_MID 90   // the middle angle of the servo
#define SERVO_MAX 180  // the maximum angle of the servo

Servo myservo; // servo object
// twelve servo objects can be created on most boards

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

void setup()
{
    Serial.begin(9600);

    // initialize the gyroscope
    move.initGyro();

    myservo.attach(SERVO_PIN); // attaches the servo on servo_pin to the servo object
    myservo.write(pos);        // tell servo to go to position in variable 'pos'

    pinMode(ECHO_PIN, INPUT);  // We receive input from the echo pin (World -> Arduino)
    pinMode(TRIG_PIN, OUTPUT); // We output to the trig pin (Arduino -> World)

    move.setServo(myservo); // set the servo
    move.setHC(hc);         // set the ultrasonic sensor

    Serial.println("Arduino ready");
}

void loop()
{
    Serial.println("dist=" + String(move.hc->dist()));
    delay(100);
}
