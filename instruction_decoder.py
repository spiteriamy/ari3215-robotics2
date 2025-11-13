'''
Turn Left::=        Point left 👈
Turn Right::=       Point right 👉
Move forward::=     Point up 👆
Move backward::=    Point down 👇
Stop::=             Closed fist ✊
Secret::=           ??? 🤔💭
'''
from enum import Enum
import mediapipe as mp

class robotcmd(Enum):
    left = 'left'
    right = 'right'
    fwd = 'forward'
    bwd = 'backward'
    stop = 'stop'
    secret = 'secret'

def decode_command_gesture(hand_landmarks):
    # 1ST STEP: find out which finger is open
    pass

def decode_duration_gesture(hand_landmarks):
    pass
