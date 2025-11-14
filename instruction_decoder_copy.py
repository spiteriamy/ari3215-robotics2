'''
Turn Left::=        Point left 👈
Turn Right::=       Point right 👉
Move forward::=     Point up 👆
Move backward::=    Point down 👇
Stop::=             Closed fist ✊
Secret::=           ??? 🤔💭
'''
from enum import Enum
import math
from mediapipe.python.solutions import hands as mp_hands

class robotcmd(Enum):
    left = 'left'
    right = 'right'
    fwd = 'forward'
    bwd = 'backward'
    stop = 'stop'
    secret = 'secret'

def decode_command_gesture(left_hand: dict[str, list[tuple[float, float, float]]]):
    # 1ST STEP: find out which finger is open
    open_finger: dict = {
        'name': None,
        'distance': 0.0,
        'points': []
    }

    for f_name, f_pts in left_hand.items(): # loop for each finger in the hand
        # calculate distance from the knuckle to the fingertip
        tip_to_knuckle = euclid_d(f_pts[-1], f_pts[0])
        
        # if this distance is greater than the previous max, update
        if open_finger['name'] is None or open_finger['distance'] < tip_to_knuckle:
            open_finger = {
                'name': f_name,
                'distance': tip_to_knuckle,
                'points': list(f_pts)
            }
        
    # print(f"open finger: {open_finger['name']}")

    # detect finger pointing direction with the angle
    mcp = open_finger['points'][0]
    tip = open_finger['points'][-1]

    angle = get_finger_angle(mcp, tip)
    # print(f"Direction angle = {angle:.2f}")

    if -120 < angle < -60:
        print("finger poitning up")

    if -30 < angle < 30:
        print("pointing right")

    if angle > 150 or angle < -150:
        print("poitning left")

    if 60 < angle < 120:
        print("pointing down")
        

def euclid_d(p1, p2) -> float:

    if len(p1) != len(p2):
        raise ValueError("Points must have the same dimension")

    sum_sq = 0.0
    for i in range(len(p1)):
        sum_sq += (p1[i]-p2[i])**2
    return sum_sq**0.5

def decode_duration_gesture(hand_landmarks):
    pass



def get_finger_angle(mcp, tip):
    # angle 0 = pointing right
    # angle 90 = pointing down
    # angle -90 = pointing up
    dx = tip[0] - mcp[0]
    dy = tip[1] - mcp[1]

    angle = math.degrees(math.atan2(dy, dx))
    return angle # in degrees
