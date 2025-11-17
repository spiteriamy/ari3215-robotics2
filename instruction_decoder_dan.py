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

# THRESHOLDS HYPERPARAMS
THRESH_FINGER_OPEN = 0.6  # distance ratio to consider finger open
THRESH_THUMB_OPEN = 0.8  # distance ratio to consider thumb open
THRESH_FIST_CLOSED = 0.7  # max distance ratio to consider fist closed


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

    for f_name, f_pts in left_hand.items():  # loop for each finger in the hand
        # calculate distance from the knuckle to the fingertip
        tip_to_wrist = euclid_d(f_pts[-1], left_hand['wrist'][0])

        # if this distance is greater than the previous max, update
        if open_finger['name'] is None or open_finger['distance'] < tip_to_wrist:
            open_finger = {
                'name': f_name,
                'distance': tip_to_wrist,
                'points': list(f_pts)
            }

    # print(f'open: {f_name}')
    print(f"open finger: {open_finger['name']}")


def euclid_d(p1, p2) -> float:

    if len(p1) != len(p2):
        raise ValueError("Points must have the same dimension")

    sum_sq = 0.0
    for i in range(len(p1)):
        sum_sq += (p1[i]-p2[i])**2
    return sum_sq**0.5


def decode_duration_gesture(hand_landmarks) -> int:
    # to count open maybe get dist from wrist->mcp->pip->dip->tip, then compare to straight wrist->tip

    # var to store no. of open fingers
    open_fingers: int = 0

    for f_name, f_pts in hand_landmarks.items():
        if f_name == 'wrist':
            continue

        # get finger lengths
        finger_length = euclid_d(f_pts[0], hand_landmarks['wrist'][0])
        for i in range(len(f_pts)-1):
            finger_length += euclid_d(f_pts[i], f_pts[i+1])

        # get straight distance wrist->tip
        wrist_to_tip = euclid_d(f_pts[-1], hand_landmarks['wrist'][0])

        # calculate ratio
        ratio = wrist_to_tip / finger_length

        # compare to threshold
        if f_name == 'thumb':
            if ratio > THRESH_THUMB_OPEN:
                open_fingers += 1
        elif ratio > THRESH_FINGER_OPEN:
            open_fingers += 1

    return open_fingers
