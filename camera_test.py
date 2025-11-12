import cv2

# For USB camera, usually index 0 or 1
cap = cv2.VideoCapture(1)

# If it's a PiCamera using libcamera, try:
# cap = cv2.VideoCapture("libcamerasrc ! videoconvert ! appsink", cv2.CAP_GSTREAMER)

if not cap.isOpened():
    print("Cannot open camera")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break

    # Process frame here (e.g. object detection)
    cv2.imshow('Video Feed', frame)

    # Press q to quit
    if cv2.waitKey(1) == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
