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

MovementSet move(100); // robot movement control

void obey(int left, int right)
{
    switch (left)
    {
        case 0: // stop
            move.stopMov();
            break;
        case 1: // forward
            move.uniformMov(1);
            break;
        case 2: // backward
            move.uniformMov(-1);
            break;
        case 3: // turn left
            move.turn(-90.0f);
            break;
        case 4: // turn right
            move.turn(90.0f);
            break;
        case 5: // secret
            break;
    }
}

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

void loop() {
    if (Serial.available() > 0) { 
        String data = Serial.readStringUntil('\n');

        int commaIndex = data.indexOf(',');
        if (commaIndex == -1) return;

        int cmd = data.substring(0, commaIndex).toInt();
        int value = data.substring(commaIndex + 1).toInt();

        // send commands to obey
        obey(cmd, value);

        // debug
        Serial.print("Received: ");
        Serial.print(cmd);
        Serial.print(" , ");
        Serial.println(value);
    }
}
