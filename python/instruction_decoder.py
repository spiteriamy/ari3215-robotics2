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

DEBUG = False


class robotcmd(Enum):
    stop = 0
    fwd = 1
    bwd = 2
    left = 3
    right = 4
    secret = 5
    nocmd = -1


def decode_command_gesture(left_hand: dict[str, list[tuple[float, float, float]]]) -> robotcmd:

    # FIRST STEP: count open fingers
    open_fingers = count_fingers(left_hand)
    print(open_fingers, end=' ') if DEBUG else None

    # if all fingers are closed -> fist
    if open_fingers == 0:
        print('stop') if DEBUG else None
        return robotcmd.stop  # return stop command

    # if all fingers are open -> secret
    elif open_fingers == 5:
        print('secret') if DEBUG else None
        return robotcmd.secret  # return secret command

    # if 2-4 fingers are open -> no command
    elif 4 >= open_fingers >= 2:
        print('no command') if DEBUG else None
        return robotcmd.nocmd  # return no command

    # 2ND STEP: find out which finger is open
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

    # 3RD STEP: find which direction the open finger is pointing

    # detect finger pointing direction with the angle
    angle = get_finger_angle(
        open_finger['points'][0], open_finger['points'][-1])

    # return command based on angle
    if -120 < angle < -45:
        print('forward') if DEBUG else None
        return robotcmd.fwd

    if -45 < angle < 45:
        print('right') if DEBUG else None
        return robotcmd.right

    if angle > 135 or angle < -120:
        print('left') if DEBUG else None
        return robotcmd.left

    if 45 < angle < 135:
        print('backward') if DEBUG else None
        return robotcmd.bwd

    return robotcmd.nocmd


def decode_commands_with_angle(left_hand: dict[str, list[tuple[float, float, float]]], right_hand: dict[str, list[tuple[float, float, float]]]) -> tuple[robotcmd, int]:

    # get the command from the left hand
    cmd: robotcmd = decode_command_gesture(left_hand)

    # count open fingers on right hand
    right_fingers_open = count_fingers(right_hand)

    # if left or right, translate the open fingers to angles
    if cmd == robotcmd.left or cmd == robotcmd.right:
        open_finger: dict = {
            'name': None,
            'distance': 0.0,
            'points': []
        }

        for f_name, f_pts in right_hand.items():  # loop for each finger in the hand
            # calculate distance from the knuckle to the fingertip
            tip_to_wrist = euclid_d(f_pts[-1], right_hand['wrist'][0])

            # if this distance is greater than the previous max, update
            if open_finger['name'] is None or open_finger['distance'] < tip_to_wrist:
                open_finger = {
                    'name': f_name,
                    'distance': tip_to_wrist,
                    'points': list(f_pts)
                }

        # detect finger pointing direction with the angle, adjusted to 0 = up
        angle = get_finger_angle(
            open_finger['points'][0], open_finger['points'][-1]) + 90
        
        if angle > 180:
            angle -= 360  # convert to -180 to 180 range

        print(f'Angle: {angle}') if DEBUG else None

        return cmd, int(angle)  # ret command and angle

    if cmd == robotcmd.secret and right_fingers_open != 5:
        cmd = robotcmd.nocmd  # require both hands for secret

    return cmd, right_fingers_open  # ret command and duration


def decode_commands(left_hand: dict[str, list[tuple[float, float, float]]], right_hand: dict[str, list[tuple[float, float, float]]]) -> tuple[robotcmd, int]:

    # get the command from the left hand
    cmd: robotcmd = decode_command_gesture(left_hand)

    # count open fingers on right hand
    right_fingers_open = count_fingers(right_hand)

    # if left or right, translate the open fingers to angles
    if cmd == robotcmd.left or cmd == robotcmd.right:
        angle = (right_fingers_open + 1) * 30  # each finger = 30 degrees

        if cmd == robotcmd.left:
            angle = -angle  # negative for left turn

        print(f'Angle: {angle}') if DEBUG else None

        return cmd, angle  # ret command and angle

    return cmd, right_fingers_open  # ret command and duration


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
    return angle
