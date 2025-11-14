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
    # mcp = open_finger['points'][0]
    # tip = open_finger['points'][-1]

    # angle = get_finger_angle(mcp, tip)
    # print(f"Direction angle = {angle:.2f}")

    # direction = get_finger_direction(angle)
    # print(f'pointing {direction}')

    num_up = count_fingers_up(left_hand)
    print("Fingers up:", num_up)

        

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


def get_finger_direction(angle): # for testing purposes 
    if -120 < angle < -60:
        return 'up'

    if -30 < angle < 30:
        return 'right'

    if angle > 150 or angle < -150:
        return 'left'

    if 60 < angle < 120:
        return 'down'


def count_fingers_up(hand: dict[str, list[tuple[float, float, float]]]):
    fingers_up = 0

    palm_center = compute_palm_center(hand)

    for name, pts in hand.items():
        if name == "wrist": # skip the wrist
            continue

        mcp = pts[0]
        pip = pts[1]
        tip = pts[-1]

        # special handling for thumb since it also points sideways
        if name == "thumb":
            # thumb is up if it points horizontally away from palm
            # works only when thumb is extended horizontally, not straight up
            # if abs(tip[0] - mcp[0]) > abs(tip[1] - mcp[1]):
            #     fingers_up += 1

            # thumb extended when palm faces camera
            # works only when palm faces cam (thumb extended to the right)
            # if tip[0] > pip[0]:
            #     fingers_up += 1

            # sometimes counts the thumb as up when its not
            if thumb_up(pts, palm_center):
                fingers_up += 1
        else:
            # normal vertical finger detection
            if tip[1] < pip[1] < mcp[1]:
                fingers_up += 1

    return fingers_up


def thumb_up(thumb_pts: list[tuple[float, float, float]], palm_center: tuple[float, float, float]) -> bool:
    mcp = thumb_pts[0]
    tip = thumb_pts[-1]

    # calculate the distance of the thumb from the center of the palm
    d_mcp = euclid_d(mcp, palm_center)
    d_tip = euclid_d(tip, palm_center)

    # how much further the tip is than the mcp
    extra = d_tip - d_mcp
    # print(extra)

    # +ve means the thumb is up -ve means thumb not up
    return extra > 0.0


def compute_palm_center(hand: dict[str, list[tuple[float, float, float]]]):
    palm_points = []

    # wrist
    if "wrist" in hand:
        palm_points.append(hand["wrist"][0])

    # mcp joints for all fingers
    for fname in ["thumb", "index", "middle", "ring", "pinky"]:
        if fname in hand:
            palm_points.append(hand[fname][0])

    # average x, y, z
    n = len(palm_points)
    cx = sum(p[0] for p in palm_points) / n
    cy = sum(p[1] for p in palm_points) / n
    cz = sum(p[2] for p in palm_points) / n

    return (cx, cy, cz)

