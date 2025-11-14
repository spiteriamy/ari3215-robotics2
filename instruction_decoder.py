'''
Turn Left::=        Point left 👈
Turn Right::=       Point right 👉
Move forward::=     Point up 👆
Move backward::=    Point down 👇
Stop::=             Closed fist ✊
Secret::=           ??? 🤔💭
'''
from enum import Enum
from mediapipe.python.solutions import hands as mp_hands

class robotcmd(Enum):
    left = 'left'
    right = 'right'
    fwd = 'forward'
    bwd = 'backward'
    stop = 'stop'
    secret = 'secret'

def decode_command_gesture(hand_landmarks):
    # 1ST STEP: find out which finger is open
    open_finger = None
    for finger in hand_landmarks: # loop for each finger in the hand
        # calculate distance from the knuckle to the fingertip
        tip_to_knuckle = euclid_d(finger[mp_hands.HandLandmark[finger.name + '_TIP']], finger[mp_hands.HandLandmark[finger.name + '_MCP']])
        
        # if this distance is greater than the previous max, update
        if open_finger is None or tip_to_knuckle > open_finger[1]:
            open_finger = (finger.name, tip_to_knuckle)
        
        print(f'open{finger.name}')
        

def euclid_d(p1, p2) -> float:
    return ((p2.x - p1.x)**2 + (p2.y - p1.y)**2 + (p2.z - p1.z)**2)**0.5

def decode_duration_gesture(hand_landmarks):
    pass
