#include "movement_set.h"

MovementSet::MovementSet(int speed) : speed(speed)
{
    this->speed = speed;

    // set motor pins to output
    pinMode(this->standbyPin, OUTPUT);
    pinMode(this->rightSpeedPin, OUTPUT);
    pinMode(this->rightDirectionPin, OUTPUT);
    pinMode(this->leftSpeedPin, OUTPUT);
    pinMode(this->leftDirectionPin, OUTPUT);

    // turn off motors
    digitalWrite(this->standbyPin, LOW);
}

void MovementSet::setServo(Servo servo)
{
    this->servo = servo;
}

void MovementSet::setHC(HCSR04 &hc)
{
    this->hc = &hc;
}

void MovementSet::initGyro()
{

    // initialize gyro
    this->gyro.MPU6050_dveInit();
    delay(2000);
    this->gyro.MPU6050_calibration();
}

void MovementSet::setSpeed(int speed)
{
    this->speed = speed;
}

void MovementSet::rightMov(int direction)
{
    digitalWrite(this->standbyPin, HIGH); // turn on motors

    // get forward/backward direction
    direction < 0 ? digitalWrite(this->rightDirectionPin, LOW) : digitalWrite(this->rightDirectionPin, HIGH);
    analogWrite(this->rightSpeedPin, this->speed);
}

void MovementSet::rightMov(double speed)
{
    digitalWrite(this->standbyPin, HIGH); // turn on motors

    // get forward/backward direction
    speed < 0 ? digitalWrite(this->rightDirectionPin, LOW) : digitalWrite(this->rightDirectionPin, HIGH);
    analogWrite(this->rightSpeedPin, abs(speed));
}

void MovementSet::leftMov(int direction)
{
    digitalWrite(this->standbyPin, HIGH); // turn on motors

    // get forward/backward direction
    direction < 0 ? digitalWrite(this->leftDirectionPin, LOW) : digitalWrite(this->leftDirectionPin, HIGH);
    analogWrite(this->leftSpeedPin, this->speed);
}

void MovementSet::leftMov(double speed)
{
    digitalWrite(this->standbyPin, HIGH); // turn on motors

    // get forward/backward direction
    speed < 0 ? digitalWrite(this->leftDirectionPin, LOW) : digitalWrite(this->leftDirectionPin, HIGH);
    analogWrite(this->leftSpeedPin, abs(speed));
}

void MovementSet::turn(float angle)
{
    if (angle == 0.0)
    {
        return;
    } // no turn

    float originalYaw, targetYaw;
    this->gyro.MPU6050_dveGetEulerAngles(&originalYaw);
    targetYaw = originalYaw + angle;

    int originalSpeed = this->speed;

    while(abs(originalYaw - targetYaw) > this->turnThresh)
    {
        // debug
        Serial.print("Yaw: ");
        Serial.println(originalYaw);

        // turn
        if (angle > 0)
        {
            rightMov(-1);
            leftMov();
        }
        else
        {
            rightMov();
            leftMov(-1);
        }

        if (abs(originalYaw - targetYaw) < this->slowThresh)
        { // if were close, slow down
            stopMov();
            delay(50);
            this->setSpeed(1.0 * this->speed / 2);
        }

        this->gyro.MPU6050_dveGetEulerAngles(&originalYaw);
    }

    // // turn clockwise (right):
    // if (angle > 0)
    // {
    //     bool slowed = false;
    //     while (originalYaw < targetYaw)
    //     {
    //         // debug
    //         // Serial.print("Yaw: ");
    //         // Serial.println(originalYaw);

    //         // turn
    //         rightMov(-1);
    //         leftMov();

    //         if (abs(originalYaw - targetYaw) < 20 && !slowed)
    //         { // if the angle is too far, turn faster
    //             stopMov();
    //             slowed = true;
    //             delay(50);
    //             this->setSpeed(1 * originalSpeed / 2);
    //         }

    //         this->gyro.MPU6050_dveGetEulerAngles(&originalYaw);
    //     }
    // }
    // // or turn anti-clockwise (left):
    // else
    // {
    //     bool slowed = false;
    //     while (originalYaw > targetYaw)
    //     {
    //         // debug
    //         // Serial.print("Yaw:");
    //         // Serial.println(originalYaw);

    //         // turn
    //         rightMov();
    //         leftMov(-1);

    //         if (abs(originalYaw - targetYaw) < 20 && !slowed)
    //         { // if were close, slow down
    //             stopMov();
    //             slowed = true;
    //             delay(50);
    //             this->setSpeed(1 * originalSpeed / 2);
    //         }
    //         // if the angle is too far, turn faster

    //         this->gyro.MPU6050_dveGetEulerAngles(&originalYaw);
    //     }
    // }

    // stop motors
    stopMov();

    // DO NOT reset yaw
    // resetYaw();

    // reset speed
    this->setSpeed(originalSpeed);
}

void MovementSet::wideTurn(float angle)
{
    Serial.println("Wide turn");

    if (angle == 0.0)
    {
        return;
    } // no turn

    float originalYaw, targetYaw;
    this->gyro.MPU6050_dveGetEulerAngles(&originalYaw);
    targetYaw = originalYaw + angle;

    int originalSpeed = this->speed;

    int pos = this->servo.read(); // get the current position of the servo

    // turn clockwise (right):
    if (angle > 0)
    {
        // look to the side
        this->servo.write(0);
        double distance = hc->dist();

        int spd = this->speed;
        this->setSpeed(60);

        while (distance > 20 || distance == 0)
        {

            uniformMov(-1);

            distance = hc->dist(); // get the distance from the ultrasonic sensor
        }
        // stop motors
        stopMov();
        this->setSpeed(spd);

        // look slightly right
        this->servo.write(55);
        distance = hc->dist();

        delay(300);

        bool slowed = false;
        while (abs(originalYaw - targetYaw) > 50 && (distance > 10 || distance == 0))
        {
            Serial.println(distance);
            // debug
            // Serial.print("Yaw: ");
            // Serial.println(originalYaw);

            // turn
            rightMov(max(1.0 * this->speed / 2, 50));
            leftMov(min(2.0 * this->speed, 255));

            // if (abs(originalYaw - targetYaw) < 30 && !slowed)
            // { // if were close, slow down
            //     stopMov();
            //     slowed = true;
            //     delay(50);
            //     this->setSpeed(1.0 * this->speed / 2);
            // }

            this->gyro.MPU6050_dveGetEulerAngles(&originalYaw);

            distance = hc->dist(); // get the distance from the ultrasonic sensor
        }
    }
    // or turn anti-clockwise (left):
    else
    {

        // look slightly left
        this->servo.write(115);
        double distance = hc->dist();

        uniformMov(-1);
        delay(300);
        stopMov();

        bool slowed = false;
        while (abs(originalYaw - targetYaw) > 45 && (distance > 10 || distance == 0))
        {
            // debug
            // Serial.print("Yaw:");
            // Serial.println(originalYaw);

            // turn
            rightMov(min(2.0 * this->speed, 255));
            leftMov(max(1.0 * this->speed / 2, 50));

            if (abs(originalYaw - targetYaw) < 40 && !slowed)
            { // if the angle is too far, turn faster

                stopMov();
                slowed = true;
                delay(50);

                this->setSpeed(1 * this->speed / 2);
            }
            this->gyro.MPU6050_dveGetEulerAngles(&originalYaw);
        }
    }

    // stop motors
    this->servo.write(pos); // reset servo position
    delay(100);
    stopMov();
    this->setSpeed(originalSpeed);
}

void MovementSet::turn(int direction)
{
    if (direction == 0)
    {
        return;
    } // no turn

    leftMov(direction);
    rightMov(-direction);
}

void MovementSet::stopMov()
{
    digitalWrite(this->standbyPin, LOW); // turn off motors
    analogWrite(this->rightSpeedPin, 0); // set speed to 0
    analogWrite(this->leftSpeedPin, 0);  // set speed to 0
}

void MovementSet::uniformMov(int direction)
{

    rightMov(direction);
    leftMov(direction);
}

void MovementSet::resetYaw()
{
    this->gyro.agz = 0.0; // reset yaw
    this->gyro.gzo = 0;   // reset gyro offset
}