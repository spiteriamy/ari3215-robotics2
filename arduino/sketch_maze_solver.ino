#include "dd_set.h"
#include "movement_set.h"

#include <HCSR04.h>
#include <Servo.h>

enum Direction
{
    NORTH,
    EAST,
    SOUTH,
    WEST
};
Direction heading = NORTH; // initial heading of the robot

// ITR20001 bs
DeviceDriverSet_ITR20001 dd_set;

// ultrasonic pins:
#define TRIG_PIN 13 // the pin to send out the ultrasonic signal
#define ECHO_PIN 12 // the pin to listen for the echo of the ultrasonic signal

// servo pins and parameters:
#define SERVO_PIN 10   // the pin the servo is connected to
#define SERVO_STEP 10  // how many degrees to move the servo each time
#define SERVO_DELAY 20 // how many milliseconds to wait for the servo to reach the position
#define SERVO_MIN 0    // the minimum angle of the servo
#define SERVO_MAX 180  // the maximum angle of the servo

HCSR04 hc(TRIG_PIN, ECHO_PIN); // ultrasonic sensor

Servo myservo; // servo object
// twelve servo objects can be created on most boards

int pos = SERVO_MIN; // variable to store the servo position

int phase = 0; // phase of the robot (rightFollow -> floodfill -> a_star)

// init the motors
MovementSet move(110);

#define MAZE_ROWS 13
#define MAZE_COLS 13

bool visited[MAZE_ROWS][MAZE_COLS] = {false}; // visited array for flood fill and A* algorithm
int maze[MAZE_ROWS][MAZE_COLS];
int start_x = 6, start_y = 1;                 // starting position of the robot
int end_x = -1, end_y = -1;                   // ending position of the robot
int current_x = start_x, current_y = start_y; // current position of the robot

// right follower
void rightFollow(int maze[MAZE_ROWS][MAZE_COLS])
{
    // look for the right wall
    myservo.write(SERVO_MIN); // tell servo to go to position in variable 'pos'
    delay(abs(pos - SERVO_MIN) * 1.0 / SERVO_STEP * SERVO_DELAY);
    pos = SERVO_MIN; // update the servo position

    double distance = hc.dist(); // get the distance from the ultrasonic sensor

    move.stopMov(); // todo: remove maybe idk adpiasdhf[aod] maybe good

    // too far -> adjust right
    if ((distance > 14 && distance < 35))
    {
        // move.turn(3.0f); // turn right

        // slow inner wheel down
        int wheel_speed = move.speed;
        move.stopMov(); // stop the robot

        move.leftMov(min(1.0 * wheel_speed, 255));     // slow down left wheel
        move.rightMov(max(1.0 * wheel_speed / 2, 50)); // keep right wheel speed
    }
    // wayy too far -> 90 deg right
    else if (distance >= 35 || distance == 0) // 40cm is the size of a cell in the maze
    {
        move.stopMov();  // stop the robot
        move.wideTurn(); // turn right
    }
    // too close -> adjust left
    else if (distance < 11)
    {
        // move.turn(-float(45.0 / distance)); // turn left

        // slow outer wheel down
        int wheel_speed = move.speed;
        move.stopMov(); // stop the robot

        move.leftMov(max(1.0 * wheel_speed / 2, 50)); // slow down left wheel
        move.rightMov(min(1.0 * wheel_speed, 255));   // keep right wheel speed
    }

    // look forward:
    myservo.write(90); // tell servo to look forward
    delay(abs(pos - 90.0) / SERVO_STEP * SERVO_DELAY);
    pos = 90.0; // update the servo position

    // get the distance
    distance = hc.dist();

    // Serial.println(distance);
    // avoid front wall if it is too close
    if (distance < 10 && distance != 0)
    {
        move.stopMov(); // stop the robot

        // add wall to the maze
        switch (heading)
        {
        case NORTH:
            maze[current_x - 1][current_y] = 1;
            break;
        case EAST:
            maze[current_x][current_y + 1] = 1;
            break;
        case SOUTH:
            maze[current_x + 1][current_y] = 1;
            break;
        case WEST:
            maze[current_x][current_y - 1] = 1;
            break;
        }

        move.turn(-90.0f); // turn left
    }
    // otherwise move forwards
    else
    {
        move.uniformMov(); // go straight

        switch (heading)
        {
        case NORTH:
            current_x--;
            break;
        case EAST:
            current_y++;
            break;
        case SOUTH:
            current_x++;
            break;
        case WEST:
            current_y--;
            break;
        }
    }
}

void visitCell()
{
    // move robot forward
    move.uniformMov(); // go straight
    switch (heading)
    {
    case NORTH:
        current_x--;
        break;
    case EAST:
        current_y++;
        break;
    case SOUTH:
        current_x++;
        break;
    case WEST:
        current_y--;
        break;
    }
    delay(750);
    move.stopMov();
}

void explore(int maze[MAZE_ROWS][MAZE_COLS])
{
    // look left
    myservo.write(SERVO_MAX);
    delay(abs(pos - SERVO_MAX) / SERVO_STEP * SERVO_DELAY);
    pos = SERVO_MAX; // update the servo position

    double left_distance = hc.dist(); // get the distance from the ultrasonic sensor

    // look forward
    myservo.write(90); // tell servo to look forward
    delay(abs(pos - 90.0) / SERVO_STEP * SERVO_DELAY);
    pos = 90.0;                          // update the servo position
    double forward_distance = hc.dist(); // get the distance from the ultrasonic sensor

    // look right
    myservo.write(SERVO_MIN); // tell servo to go to position in variable 'pos'
    delay(abs(pos - SERVO_MIN) / SERVO_STEP * SERVO_DELAY);
    pos = SERVO_MIN;                   // update the servo position
    double right_distance = hc.dist(); // get the distance from the ultrasonic sensor

    Serial.print("Left: ");
    Serial.print(left_distance);
    Serial.print(" Forward: ");
    Serial.print(forward_distance);
    Serial.print(" Right: ");
    Serial.println(right_distance);



    if (right_distance > 22 || right_distance == 0)
    {
        // turn right
        move.turn(90.0f);
    }
    // else if (left_distance > 14 && left_distance < 35)
    // {
    //     // turn left
    //     move.turn(-90.0f);
    // }
    else if (forward_distance < 14 && forward_distance != 0)
    {
        // turn left
        move.turn(-90.0f);
    }
    else
    {
        // move forward
        visitCell();
    }
}

// flood fill function
void floodfill(int start_x, int start_y, int end_x, int end_y, int maze[MAZE_ROWS][MAZE_COLS], int visited[MAZE_ROWS][MAZE_COLS])
{
    // check if the current position is out of bounds or already visited
    if (start_x < 0 || start_x >= MAZE_ROWS || start_y < 0 || start_y >= MAZE_COLS || visited[start_x][start_y] == 1)
        return;

    // mark the current position as visited
    visited[start_x][start_y] = 1;

    // check if we have reached the end position
    if (start_x == end_x && start_y == end_y)
        return;

    // rest of the code ...
}

// A* function
void a_star(int start_x, int start_y, int end_x, int end_y, int maze[MAZE_ROWS][MAZE_COLS], int visited[MAZE_ROWS][MAZE_COLS])
{
    // check if the current position is out of bounds or already visited
    if (start_x < 0 || start_x >= MAZE_ROWS || start_y < 0 || start_y >= MAZE_COLS || visited[start_x][start_y] == 1)
        return;

    // mark the current position as visited
    visited[start_x][start_y] = 1;

    // check if we have reached the end position
    if (start_x == end_x && start_y == end_y)
        return;

    // rest of the code ...
}

void printMaze(int maze[MAZE_ROWS][MAZE_COLS])
{
    for (int i = 0; i < MAZE_ROWS; i++)
    {
        for (int j = 0; j < MAZE_COLS; j++)
        {
            Serial.print(maze[i][j]);
            Serial.print(" ");
        }
        Serial.println();
    }
}
void initMaze(int maze[MAZE_ROWS][MAZE_COLS])
{
    // initialize the maze with zeros
    for (int i = 0; i < MAZE_ROWS; i++)
    {

        for (int j = 0; j < MAZE_COLS; j++)
        {
            if (i == 0 || i == MAZE_ROWS - 1 || j == 0 || j == MAZE_COLS - 1)
            {
                maze[i][j] = 1;
            }
            else
            {
                maze[i][j] = 0;
            }
        }
    }

    Serial.println("Maze initialized:");
    printMaze(maze);
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

    initMaze(maze); // initialize the maze

    // line following sensors:
    dd_set.DeviceDriverSet_ITR20001_Init();
}
void loop()
{
    // myservo.write(SERVO_MIN); // tell servo to go to position in variable 'pos'
    // delay(abs(pos - SERVO_MIN) * 1.0 / SERVO_STEP * SERVO_DELAY);
    // pos = SERVO_MIN; // update the servo position

    // double distance = hc.dist(); // get the distance from the ultrasonic sensor
    // Serial.println(distance);

    if (phase == 0)
    {
        // rightFollow(maze);
        explore(maze);
    }

    // print the maze
    // static unsigned long printTime = 0;
    // if (millis() - printTime > 1000)
    // {
    //     printTime = millis();
    //     printMaze(maze);
    // }

    // read the line following sensors
    // int left = dd_set.DeviceDriverSet_ITR20001_getAnaloguexxx_L();
    // int middle = dd_set.DeviceDriverSet_ITR20001_getAnaloguexxx_M();
    // int right = dd_set.DeviceDriverSet_ITR20001_getAnaloguexxx_R();

    // white: 130 50 50
    // gray: 730 600 670
    // black: 890 820 860
    // if (left > 700 && middle > 700 && right > 700)
    // {
    //     // all sensors are on the finish
    //     // stop the robot
    //     move.stopMov();
    //     phase = 1; // go to the next phase
    // }
    // else if (left > 500 && middle > 500 && right > 500)
    // {
    //     // all sensors are on the start
    //     // do nothing for now
    // }

    // move.wideTurn();
    // delay(1000);

    move.stopMov(); // stop the robot
}

// speed 75 delay 1300