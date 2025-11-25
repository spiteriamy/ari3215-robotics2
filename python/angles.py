#!/usr/bin/env python3
import time
import serial

# initialise serial for arduino communication
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(5) # wait for arduino
ser.reset_input_buffer()
print("Arduino:", ser.readline().decode('utf-8').strip())
last_send = 0

# send test commands without hand detection

messages = [
    f"{3},{30}\n", # left 30
    f"{4},{30}\n", # right 30
    f"{3},{90}\n", # left 90
    f"{4},{90}\n", # right 90
    f"{3},{180}\n", # left 180
    f"{4},{180}\n" # right 180
]

for message in messages:
    ser.write(message.encode())
    print("Sent:", message.strip())

    reply = ser.readline().decode().strip()
    if reply:
        print("Arduino:", reply)

    time.sleep(10)

