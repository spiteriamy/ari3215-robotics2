# Hand Gesture Controlled Robot - Group Project

Group project developed for ARI3215 - Robotics 2, focused on designing and implementing an autonomous robotic system that exhibits intelligent behaviour.

---

## Team Members

| Name | GitHub |
|------|--------|
| Amy Spiteri | [spiteriamy](https://github.com/spiteriamy) |
| Clyde Vella | [Clyde2105](https://github.com/Clyde2105) |
| Daniel Pace | [funtimezallaround](https://github.com/funtimezallaround) |

---

## Table of Contents

- [About the Project](#about-the-project)
- [Features](#features)
- [Tech Stack](#tech-stack)
- [Getting Started](#getting-started)
  - [Hardware Setup](#hardware-setup)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
  - [Usage](#usage)
- [Command Gestures](#command-gestures)
- [Project Structure](#project-structure)

---

## About the Project

Our project is an implementation of a gesture-controlled robot that responds to hand gestures captured by a webcam. The system uses computer vision techniques to recognize specific gestures made by the user and translates them into movement commands for the robot. For example, gestures such as a closed fist, or other finger positions instruct the robot to move forward, backward, turn, or stop (see [Command Gestures](#command-gestures) for table of commands). The robot also incorporates an ultrasonic sensor to detect nearby obstacles and prevent collisions, and a gyroscope for angle accuracy in turning.

---

## Features

- Acquires real-time visual input from a webcam. 
- Processes this input using computer vision techniques to detect and classify hand gestures. 
- Maps each recognized gesture to a corresponding robot movement command. (see [Command Gestures](#command-gestures) for table of commands)
- Integrates obstacle detection (with the ultrasonic sensor) and motion stabilization (with the gyroscope) for safer and more accurate movement.

---

## Tech Stack

**Languages:**  
- Python (Version 3.10)
- C++

**Tools & Libraries:**  
- OpenCV
- MediaPipe
- pySerial
- Arduino
- HCSR04
- MPU6050

---

## Getting Started

### Hardware Setup

- Connect the USB camera and Arduino to the correct ports on the Raspberry Pi
- Position the camera facing up properly in the correct direction

The robot setup can be seen as pictured below:

[add image]

### Prerequisites

To run this project install all of the dependencies in [requirements.txt](python/requirements.txt):

```bash
pip install -r requirements.txt
```

### Installation

```bash
git clone https://github.com/spiteriamy/ari3215-robotics2.git
cd ari3215-robotics2/python
chmod +x main.py
```

### Usage

First flash the Arduino program [robot.ino](arduino/robot/robot.ino) onto the Arduino board.

Then run the Python program [main.py](python/main.py) on the Raspberry Pi:

```bash
cd python
./main.py
```

---

## Command Gestures

### **Left Hand — Directional Commands**

| Gesture     | Example | Action         |
| ----------- | ------ | -------------- |
| Point left  | 👈     | Turn Left      |
| Point right | 👉     | Turn Right     |
| Point up    | 👆     | Move Forward   |
| Point down  | 👇     | Move Backward  |
| Closed fist | ✊     | Stop           |
| Open hand   | 🖐️     | Secret         |

### **Right Hand — Duration & Angle Controls**

| Gesture Type                     | Meaning                               | Example                                          |
| -------------------------------- | ------------------------------------- | ------------------------------------------------ |
| **Fingers for Forward/Backward** | Number of fingers = number of seconds | 3 fingers → move for 3 seconds                   |
| **Fingers for Turning**          | Number of fingers = turning angle (base 30° + 30° per finger)   | 0 fingers = 30°, 1 finger = 60°, 2 fingers = 90° |
| **Closed fist**                  | Stop                                  | ✊                                                |
| **Open hand**                    | Secret                        | 🖐️                                              |

---

## Project Structure

```css
ari3215-robotics2/
│── arduino/
│   ├── gyrotest/
│   └── robot/
│       ├── movement_set.cpp
│       ├── movement_set.h
│       └── robot.ino
│
│── documentation/
│
│── python/
│   ├── camera_test.py
│   ├── instruction_decoder.py
│   ├── main.py
│   └── requirements.txt
│
└── README.md
```

- [**arduino**](arduino/): Contains all Arduino code.
  - [**gyrotest**](arduino/gyrotest/): To test the turning by specific angles using the gyroscope.
  - [**robot**](arduino/robot/): Final program to be used on the robot.
    - [**movement_set.h**](arduino/robot/movement_set.h), [**movement_set.cpp**](arduino/robot/movement_set.cpp): Robot movement control.
    - [**robot.ino**](arduino/robot/robot.ino): Main program that receives the commands from the Raspberry Pi.

- [**python**](python/): Contains all of the Python scripts to be ran on the Raspberry Pi.
  - [**instruction_decoder.py**](python/instruction_decoder.py): Maps the hand gestures to the robot commands.
  - [**main.py**](python/main.py): Main program that gets the webcam input and sends the commands to the Arduino.

---
