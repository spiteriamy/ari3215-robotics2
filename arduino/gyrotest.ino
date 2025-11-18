#include "movement_set.h"

#include <HCSR04.h>
#include <Servo.h>

// ultrasonic pins:
#define TRIG_PIN 13 // the pin to send out the ultrasonic signal
#define ECHO_PIN 12 // the pin to listen for the echo of the ultrasonic signal

HCSR04 hc(TRIG_PIN, ECHO_PIN); // ultrasonic sensor

// servo pins and parameters:
#define SERVO_PIN 10   // the pin the servo is connected to
#define SERVO_STEP 10  // how many degrees to move the servo each time
#define SERVO_DELAY 20 // how many milliseconds to wait for the servo to reach the position
#define SERVO_MIN 0    // the minimum angle of the servo
#define SERVO_MAX 180  // the maximum angle of the servo

Servo myservo; // servo object
// twelve servo objects can be created on most boards

int pos = SERVO_MIN; // variable to store the servo position

MovementSet move(100);

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
}

void loop()
{
    move.turn(90.0f);
    delay(2000);
}
