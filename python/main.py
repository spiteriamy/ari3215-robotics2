#!/usr/bin/env python3
import cv2
import mediapipe as mp
import time
import serial
from mediapipe.python.solutions import drawing_utils as mp_drawing
from mediapipe.python.solutions import drawing_styles as mp_drawing_styles
from mediapipe.python.solutions import hands as mp_hands
import instruction_decoder as dc


# choose between webcam or video file
# 1 for webcam, or provide video file path
# VIDEO_PATH: int | str = 'handy_video.mp4'
VIDEO_PATH = 1

# Open video source
cap = cv2.VideoCapture(VIDEO_PATH)

if not cap.isOpened():
    print(f"Error: Could not open video file: {VIDEO_PATH}")
    exit()


# initialise serial for arduino communication
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(5) # wait for arduino
ser.reset_input_buffer()
print("Arduino:", ser.readline().decode('utf-8').strip())
last_send = 0

with mp_hands.Hands() as hands:

    while cap.isOpened() or isinstance(VIDEO_PATH, str):
        success, image = cap.read()
        if not success:
            if isinstance(VIDEO_PATH, str):
                print("End of video file reached.")
                cap.set(cv2.CAP_PROP_POS_FRAMES, 0)  # Reset to first frame
            else:
                print("Ignoring empty camera frame.")
            continue

        # Flip the image horizontally for natural (selfie) view
        image = cv2.flip(image, 1)
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

        # Process image for hand detection
        results = hands.process(image_rgb)

        left_hand = None
        right_hand = None

        if results.multi_hand_landmarks and results.multi_handedness:  # type: ignore
            for hand_landmarks, handedness in zip(
                reversed(results.multi_hand_landmarks), reversed(  # type: ignore
                    results.multi_handedness)  # type: ignore
            ):
                label = handedness.classification[0].label  # "Left" or "Right"

                # Save only one of each side
                if label == "Left" and left_hand is None:
                    left_hand = hand_landmarks
                elif label == "Right" and right_hand is None:
                    right_hand = hand_landmarks

        if left_hand and right_hand:

            # Draw the detected hands
            # mp_drawing.draw_landmarks(
            #     image,
            #     left_hand,
            #     list(mp_hands.HAND_CONNECTIONS),
            #     mp_drawing_styles.get_default_hand_landmarks_style(),
            #     mp_drawing_styles.get_default_hand_connections_style(),
            # )
            # mp_drawing.draw_landmarks(
            #     image,
            #     right_hand,
            #     list(mp_hands.HAND_CONNECTIONS),
            #     mp_drawing_styles.get_default_hand_landmarks_style(),
            #     mp_drawing_styles.get_default_hand_connections_style(),
            # )

            # decode hand instruction
            for landmark in left_hand.landmark:
                landmark.x *= cap.get(cv2.CAP_PROP_FRAME_WIDTH)
                landmark.y *= cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
                landmark.z *= cap.get(cv2.CAP_PROP_FRAME_WIDTH)

            left_hand_obj: dict[str, list[tuple[float, float, float]]] = {
                'wrist': [
                    (left_hand.landmark[0].x,
                     left_hand.landmark[0].y,
                     left_hand.landmark[0].z)
                ],
                'thumb': [
                    (left_hand.landmark[i].x,
                     left_hand.landmark[i].y,
                     left_hand.landmark[i].z)
                    for i in range(1, 5)
                ],
                'index': [
                    (left_hand.landmark[i].x,
                     left_hand.landmark[i].y,
                     left_hand.landmark[i].z)
                    for i in range(5, 9)
                ],
                'middle': [
                    (left_hand.landmark[i].x,
                     left_hand.landmark[i].y,
                     left_hand.landmark[i].z)
                    for i in range(9, 13)
                ],
                'ring': [
                    (left_hand.landmark[i].x,
                     left_hand.landmark[i].y,
                     left_hand.landmark[i].z)
                    for i in range(13, 17)
                ],
                'pinky': [
                    (left_hand.landmark[i].x,
                     left_hand.landmark[i].y,
                     left_hand.landmark[i].z)
                    for i in range(17, 21)
                ],
            }

            for landmark in right_hand.landmark:
                landmark.x *= cap.get(cv2.CAP_PROP_FRAME_WIDTH)
                landmark.y *= cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
                landmark.z *= cap.get(cv2.CAP_PROP_FRAME_WIDTH)

            right_hand_obj: dict[str, list[tuple[float, float, float]]] = {
                'wrist': [
                    (right_hand.landmark[0].x,
                     right_hand.landmark[0].y,
                     right_hand.landmark[0].z)
                ],
                'thumb': [
                    (right_hand.landmark[i].x,
                     right_hand.landmark[i].y,
                     right_hand.landmark[i].z)
                    for i in range(1, 5)
                ],
                'index': [
                    (right_hand.landmark[i].x,
                     right_hand.landmark[i].y,
                     right_hand.landmark[i].z)
                    for i in range(5, 9)
                ],
                'middle': [
                    (right_hand.landmark[i].x,
                     right_hand.landmark[i].y,
                     right_hand.landmark[i].z)
                    for i in range(9, 13)
                ],
                'ring': [
                    (right_hand.landmark[i].x,
                     right_hand.landmark[i].y,
                     right_hand.landmark[i].z)
                    for i in range(13, 17)
                ],
                'pinky': [
                    (right_hand.landmark[i].x,
                     right_hand.landmark[i].y,
                     right_hand.landmark[i].z)
                    for i in range(17, 21)
                ],
            }

            cmd, value = dc.decode_commands(left_hand_obj, right_hand_obj)
            print((cmd, value))

            # format command for arduino
            cmd = cmd.value

            message = None

            if cmd == -1:
                message = None
            elif cmd == 5 and value != 5:
                message = None
            elif cmd == 0 and value != 0:
                message = None
            else:
                message = f"{cmd},{value} \n"

            # instead of sending every time hands are detected
            # so arduino can keep up
            send_thresh = 5
            if time.time() - last_send > send_thresh and cmd != -1 and message != None: 
                print(f"DEBUG: cmd={cmd}, value={value}, message={message.strip()}")
                print("message (raw):", message.encode())
                ser.write(message.encode())
                ser.flush()
                print("Sent:", message.strip())
                last_send = time.time()

                # read Arduino reply
                reply = ser.readline().decode().strip()
                if reply:
                    print("Arduino:", reply)


        # Display results
        # cv2.imshow("Hand Detection (1 Left + 1 Right)", image)

        if cv2.waitKey(5) & 0xFF == 27:  # ESC to exit
            break

cap.release()
cv2.destroyAllWindows()
