/*
 * @file movement_set.h
 * @brief Header file for the MovementSet class, which controls the movement of the ELEGOO robot.
 *
 * This file contains the declaration of the MovementSet class, which provides methods to control
 * the movement of the ELEGOO robot. The class includes methods for moving forward, backward, turning left,
 * turning right, and stopping the robot.
 *
 * @date 14/05/2025
 */

#ifndef MOVEMENT_SET_H
#define MOVEMENT_SET_H
#include <Arduino.h>
#include <Servo.h>
#include <HCSR04.h>
#include "MPU6050_getdata.h"

class MovementSet
{
private:
    // motor pins:

    const int standbyPin = 3; // on/off switch for motors

    // right:
    const int rightSpeedPin = 5;     // speed control for right motors
    const int rightDirectionPin = 7; // direction control for right motors

    // left:
    const int leftSpeedPin = 6;     // speed control for left motors
    const int leftDirectionPin = 8; // direction control for left motors

    // private method to reset the yaw
    void resetYaw();

public:
    int speed;              // default speed of the robot
    float turnThresh = 0.1; // acceptable error for turning
    int slowThresh = 20;    // threshold to start slowing down when turning

    bool HALT = false; // emergency stop flag

    MPU6050_getdata gyro;
    Servo servo; // servo object
    HCSR04 *hc;  // ultrasonic sensor object

    // constructor
    MovementSet(int speed = 100);

    void setServo(Servo servo);
    void setHC(HCSR04 &hc);

    void initGyro();
    // set the speed of the robot
    // speed is a value between 0 and 255
    void setSpeed(int speed);

    // move the right motor at a given speed
    void rightMov(int direction = 1);
    void rightMov(double speed);

    // move the left motor at a given speed
    void leftMov(int direction = 1);
    void leftMov(double speed);
    // turn the robot at a given angle
    void turn(float angle);
    void wideTurn(float angle = 90.0f);

    // turn the robot in a given direction, built-in obstacle avoidance
    void turn(float angle);
    // turn the robot in a given direction
    void turn(int direction);

    // stop both motors
    void stopMov();

    // move both motors at the same speed
    void uniformMov(int direction = 1);

    // tweak tf out
    void tweak();

    // private method to check for obstacles and avoid them
    // @param thresh The distance threshold to consider an obstacle (in cm)
    // @param where The value to pass to the servo to check for obstacles
    void isObstacle(int thresh = 10, int where = 90);
};
#endif