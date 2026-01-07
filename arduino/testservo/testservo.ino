#include <Servo.h>
#include <HCSR04.h>

// ----------------------------
// Pins
// ----------------------------
#define TRIG_PIN 13
#define ECHO_PIN 12
#define SERVO_PIN 10

// ----------------------------
// Servo Angles
// ----------------------------
#define SERVO_LEFT 170
#define SERVO_CENTER 96
#define SERVO_RIGHT 10

// ----------------------------
// Globals
// ----------------------------
HCSR04 hc(TRIG_PIN, ECHO_PIN);
Servo servoMotor;

void setup() {
  Serial.begin(9600);
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(SERVO_CENTER); // center at start
  delay(500);
}

void loop() {
  // LEFT
  slowServoMove(SERVO_CENTER, SERVO_LEFT);
  delay(300);
  int distLeft = hc.dist();
  Serial.print("⬅️ Left Distance: ");
  Serial.print(distLeft);
  Serial.println(" cm");

  delay(500);

  // CENTER
  slowServoMove(SERVO_LEFT, SERVO_CENTER);
  delay(300);
  int distCenter = hc.dist();
  Serial.print("⬆️ Center Distance: ");
  Serial.print(distCenter);
  Serial.println(" cm");

  delay(500);

  // RIGHT
  slowServoMove(SERVO_CENTER, SERVO_RIGHT);
  delay(300);
  int distRight = hc.dist();
  Serial.print("➡️ Right Distance: ");
  Serial.print(distRight);
  Serial.println(" cm");

  delay(500);

  // BACK TO CENTER
  slowServoMove(SERVO_RIGHT, SERVO_CENTER);
  delay(1000); // pause before next sweep
}

// ----------------------------
// Smooth Servo Movement
// ----------------------------
void slowServoMove(int fromAngle, int toAngle) {
  int step = (fromAngle < toAngle) ? 1 : -1;
  for (int pos = fromAngle; pos != toAngle; pos += step) {
    servoMotor.write(pos);
    delay(10); // slows down movement
  }
  servoMotor.write(toAngle); // ensure it lands on final angle
}
