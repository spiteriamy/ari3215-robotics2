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

# THRESHOLDS HYPERPARAMS
THRESH_FINGER_OPEN = 0.6  # distance ratio to consider finger open
THRESH_THUMB_OPEN = 0.85  # distance ratio to consider thumb open
THRESH_FIST_CLOSED = 0.7  # max distance ratio to consider fist closed


class robotcmd(Enum):
    left = 'left'
    right = 'right'
    fwd = 'forward'
    bwd = 'backward'
    stop = 'stop'
    secret = 'secret'
    nocmd = '/'


def decode_command_gesture(left_hand: dict[str, list[tuple[float, float, float]]]) -> robotcmd:
    # for stop command - fist
    # finger count needs to be 0
    open_fingers = count_fingers(left_hand)
    print(open_fingers, end=' ')

    if open_fingers == 0:
        # command = stop
        print('stop')
        return robotcmd.stop
    elif open_fingers == 5:
        print('secret')
        return robotcmd.secret
    elif 4 >= open_fingers >= 2:
        print('no command')
        return robotcmd.nocmd
    
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

    # the rest of the commands depend on which 
    # finger and the direction its pointing

    # detect finger pointing direction with the angle
    mcp = open_finger['points'][0]
    tip = open_finger['points'][-1]

    angle = get_finger_angle(mcp, tip)
    direction = get_finger_direction(angle)

    if open_finger['name'] == 'thumb':
        if direction == 'left':
            # command = turn left
            print('turn left')
            return robotcmd.left
        elif direction == 'right':
            # command = turn right
            print('turn right')
            return robotcmd.right
        else:
            print('NO COMMAND')
    elif open_finger['name'] == 'index':
        if direction == 'up':
            # command = move forward
            print('move forward')
            return robotcmd.fwd
        elif direction == 'down':
            # command == move backward
            print('move backward')
            return robotcmd.bwd
        else:
            print('NO COMMAND')
    else:
        print('NO COMMAND')


def decode_duration_gesture():
    pass


def euclid_d(p1, p2) -> float:

    if len(p1) != len(p2):
        raise ValueError("Points must have the same dimension")

    sum_sq = 0.0
    for i in range(len(p1)):
        sum_sq += (p1[i]-p2[i])**2
    return sum_sq**0.5


def count_fingers(hand_landmarks) -> int:
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


def get_finger_angle(mcp, tip):
    # angle 0 = pointing right
    # angle 90 = pointing down
    # angle -90 = pointing up
    dx = tip[0] - mcp[0]
    dy = tip[1] - mcp[1]

    angle = math.degrees(math.atan2(dy, dx))
    return angle # in degrees


def get_finger_direction(angle):
    if -135 < angle < -45:
        return 'up'

    if -45 < angle < 45:
        return 'right'

    if angle > 135 or angle < -135:
        return 'left'

    if 45 < angle < 135:
        return 'down'


