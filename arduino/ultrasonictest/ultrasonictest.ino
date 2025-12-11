#include "movement_set.h"

// ultrasonic pins:
#define TRIG_PIN_1 13 // the pin to send out the ultrasonic signal
#define ECHO_PIN_1 12 // the pin to listen for the echo of the ultrasonic signal

#define BUZZER_PIN 11 // Buzzer pin

#define TRIG_PIN_2 A0 
#define ECHO_PIN_2 A1

HCSR04 hc1(TRIG_PIN_1, ECHO_PIN_1); // ultrasonic sensor
HCSR04 hc2(TRIG_PIN_2, ECHO_PIN_2); // ultrasonic sensor 2

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

    // Setup Sensor 1
    pinMode(ECHO_PIN_1, INPUT);
    pinMode(TRIG_PIN_1, OUTPUT);

    // Setup Sensor 2 (The new one)
    pinMode(ECHO_PIN_2, INPUT);
    pinMode(TRIG_PIN_2, OUTPUT);

    pinMode(BUZZER_PIN, OUTPUT);

    move.setServo(myservo); // set the servo
    // move.setHC(hc);         // set the ultrasonic sensor

    Serial.println("Arduino ready - Testing Sensor 2");
}

void loop()
{
    // Read from the second sensor directly
    int dist1 = hc1.dist();
    delay(50); // Small delay to prevent sensor interference
    int dist2 = hc2.dist();
    delay(50); // Small delay to prevent sensor interference

    Serial.println("Sensor 1 dist=" + String(dist1) + " , " + "Sensor 2 dist=" + String(dist2));
    
    if (dist1 != 0 || dist2 != 0) {
        tone(BUZZER_PIN, 500);
    }
    else {
        noTone(BUZZER_PIN);
    }
    
    delay(100); // Small delay to prevent sensor interference
}
