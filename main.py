import cv2
import mediapipe as mp

# Initialize Mediapipe modules
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_hands = mp.solutions.hands

# Open webcam
cap = cv2.VideoCapture(0)

with mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=2,  # Allow up to 2 hands
    model_complexity=1,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5
) as hands:

    while cap.isOpened():
        success, image = cap.read()
        if not success:
            print("Ignoring empty camera frame.")
            continue

        # Flip the image horizontally for natural (selfie) view
        image = cv2.flip(image, 1)
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

        # Process image for hand detection
        results = hands.process(image_rgb)

        left_hand = None
        right_hand = None

        if results.multi_hand_landmarks and results.multi_handedness:
            for hand_landmarks, handedness in zip(
                reversed(results.multi_hand_landmarks), reversed(results.multi_handedness)
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
                mp_hands.HAND_CONNECTIONS,
                mp_drawing_styles.get_default_hand_landmarks_style(),
                mp_drawing_styles.get_default_hand_connections_style(),
            )

            # decode hand instruction
            for landmark in left_hand.landmark:
                landmark.x *= cap.get(cv2.CAP_PROP_FRAME_WIDTH)
                landmark.y *= cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
                landmark.z *= cap.get(cv2.CAP_PROP_FRAME_WIDTH)
            
            print(type(left_hand))

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

            print(f"wrist: {left_hand.landmark[mp_hands.HandLandmark.WRIST].x}, index: {left_hand.landmark[mp_hands.HandLandmark.INDEX_FINGER_TIP].x}")

        if right_hand:
            mp_drawing.draw_landmarks(
                image,
                right_hand,
                mp_hands.HAND_CONNECTIONS,
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