import cv2
import mediapipe as mp
from mediapipe.python.solutions import drawing_utils as mp_drawing
from mediapipe.python.solutions import drawing_styles as mp_drawing_styles
from mediapipe.python.solutions import hands as mp_hands
# import instruction_decoder as dc
import instruction_decoder_copy as dc



# choose between webcam or video file
VIDEO_PATH: int | str = 'handy_video.mp4'  # 0 for webcam, or provide video file path

# Open video source
# cap = cv2.VideoCapture(VIDEO_PATH)
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print(f"Error: Could not open video file: {VIDEO_PATH}")
    exit()
    

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
                reversed(results.multi_hand_landmarks), reversed( # type: ignore
                    results.multi_handedness)  # type: ignore
            ):
                label = handedness.classification[0].label  # "Left" or "Right"

                # Save only one of each side
                if label == "Left" and left_hand is None:
                    left_hand = hand_landmarks
                elif label == "Right" and right_hand is None:
                    right_hand = hand_landmarks

        if left_hand:

            # Draw the detected hands
            mp_drawing.draw_landmarks(
                image,
                left_hand,
                list(mp_hands.HAND_CONNECTIONS),
                mp_drawing_styles.get_default_hand_landmarks_style(),
                mp_drawing_styles.get_default_hand_connections_style(),
            )

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

            dc.decode_command_gesture(left_hand_obj)

            # hand_base_xyz:list[float] = [
            #     left_hand.landmark[mp_hands.HandLandmark.WRIST].x*cap.get(cv2.CAP_PROP_FRAME_WIDTH),
            #     left_hand.landmark[mp_hands.HandLandmark.WRIST].y*cap.get(cv2.CAP_PROP_FRAME_HEIGHT),
            #     left_hand.landmark[mp_hands.HandLandmark.WRIST].z,
            # ]

            # index_xyz:list[float] = [
            #     left_hand.landmark[mp_hands.HandLandmark.INDEX_FINGER_TIP].x*cap.get(cv2.CAP_PROP_FRAME_WIDTH),
            #     left_hand.landmark[mp_hands.HandLandmark.INDEX_FINGER_TIP].y*cap.get(cv2.CAP_PROP_FRAME_HEIGHT),
            #     left_hand.landmark[mp_hands.HandLandmark.INDEX_FINGER_TIP].z*cap.get(cv2.CAP_PROP_FRAME_WIDTH),
            # ]

        if right_hand:
            mp_drawing.draw_landmarks(
                image,
                right_hand,
                list(mp_hands.HAND_CONNECTIONS),
                mp_drawing_styles.get_default_hand_landmarks_style(),
                mp_drawing_styles.get_default_hand_connections_style(),
            )
            # print('right hand 🤚')

            # mp_drawing.plot_landmarks(right_hand, mp_hands.HAND_CONNECTIONS)

        # Display results
        cv2.imshow("Hand Detection (1 Left + 1 Right)", image)

        if cv2.waitKey(5) & 0xFF == 27:  # ESC to exit
            break

cap.release()
cv2.destroyAllWindows()
