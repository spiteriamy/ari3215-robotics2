import cv2
import mediapipe as mp

mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_hands = mp.solutions.hands

# Track which hands are currently "locked"
locked_hands = {"Left": None, "Right": None}
hands_visible = {"Left": False, "Right": False}

cap = cv2.VideoCapture(0)

with mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=2,
    model_complexity=1,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5
) as hands:

    while cap.isOpened():
        success, image = cap.read()
        if not success:
            print("Ignoring empty camera frame.")
            continue

        image = cv2.flip(image, 1)
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        results = hands.process(image_rgb)

        # Reset visibility flags each frame
        hands_visible["Left"] = False
        hands_visible["Right"] = False

        if results.multi_hand_landmarks and results.multi_handedness:
            for hand_landmarks, handedness in zip(
                results.multi_hand_landmarks, results.multi_handedness
            ):
                label = handedness.classification[0].label  # "Left" or "Right"

                # If this side is not locked yet, lock it to this hand
                if locked_hands[label] is None:
                    locked_hands[label] = hand_landmarks
                    hands_visible[label] = True
                else:
                    # If already locked, check if it's likely the same tracked hand
                    # Mediapipe’s tracking is already stable per hand_id internally
                    hands_visible[label] = True
                    locked_hands[label] = hand_landmarks

        # If a locked hand is no longer visible, unlock it
        for side in ["Left", "Right"]:
            if not hands_visible[side]:
                locked_hands[side] = None  # Release the slot

        # Draw only the locked hands
        for side in ["Left", "Right"]:
            if locked_hands[side]:
                mp_drawing.draw_landmarks(
                    image,
                    locked_hands[side],
                    mp_hands.HAND_CONNECTIONS,
                    mp_drawing_styles.get_default_hand_landmarks_style(),
                    mp_drawing_styles.get_default_hand_connections_style(),
                )

        cv2.imshow("Stable Hand Lock (Left + Right)", image)

        if cv2.waitKey(5) & 0xFF == 27:
            break

cap.release()
cv2.destroyAllWindows()