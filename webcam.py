import numpy as np
import cv2
import matplotlib.pyplot as plt
from scipy.stats import itemfreq
from collections import deque
import os

pts = deque(maxlen=32)
counter = 0
(dX, dY) = (0, 0)
direction = ""
cap = cv2.VideoCapture(0)

while(True):
    # Capture frame-by-frame
    ret, frame = cap.read()
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    # Our operations on the frame come here
    #gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Display the resulting frame
    height, width = frame.shape[:2]
    # Center of image
    img = frame[int(height/2 - 50):int(height/2 + 50), int(width/2 - 50):int(width/2 + 50)]
    cv2.rectangle(frame, (int(width/2 - 50), int(height/2 - 50)), (int(width/2 + 50),int(height/2 + 50)), (0,255,0),3)
    cv2.imshow('crop', frame)
    # Dominant color of center, using kmeans
    # https://stackoverflow.com/questions/43111029/how-to-find-the-average-colour-of-an-image-in-python-with-opencv
    arr = np.float32(img)
    pixels = arr.reshape((-1, 3))

    n_colors = 5
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 200, .1)
    flags = cv2.KMEANS_RANDOM_CENTERS
    _, labels, centroids = cv2.kmeans(pixels, n_colors, None, criteria, 10, flags)

    palette = np.uint8(centroids)
    quantized = palette[labels.flatten()]
    quantized = quantized.reshape(img.shape)
    dominant_color = palette[np.argmax(itemfreq(labels)[:, -1])]
    blue, green, red = dominant_color
    color = np.uint8([[[blue, green, red]]])
    hsv_color = cv2.cvtColor(color, cv2.COLOR_BGR2HSV)
    hue, lum, sat = hsv_color[0][0]
    lowHue = hue - 10
    if lowHue < 0:
        lowHue = 0
    highHue = hue + 10
    if highHue > 179:
        highHue = 179
    lower_mask = np.array([lowHue, 50, 50])
    upper_mask = np.array([highHue, 255, 255])
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

while(True):
    ret, frame = cap.read()
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    # https://www.pyimagesearch.com/2015/09/21/opencv-track-object-movement/
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, lower_mask, upper_mask)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)
    cv2.imshow('mask',mask)
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE)[-2]
    center = None
    # only proceed if at least one contour was found
    if len(cnts) > 0:
        # find the largest contour in the mask, then use
        # it to compute the minimum enclosing circle and
        # centroid
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
 
        # only proceed if the radius meets a minimum size
        if radius > 10:
            # draw the circle and centroid on the frame,
            # then update the list of tracked points
            cv2.circle(frame, (int(x), int(y)), int(radius),
                (0, 255, 255), 2)
            cv2.circle(frame, center, 5, (0, 0, 255), -1)
            pts.appendleft(center)
            # print(center)

    for i in np.arange(1, len(pts)):
        # if either of the tracked points are None, ignore
        # them
        if pts[i - 1] is None or pts[i] is None:
            continue
 
        # check to see if enough points have been accumulated in
        # the buffer
        if counter >= 10 and i == 1 and pts[-10] is not None:
            # compute the difference between the x and y
            # coordinates and re-initialize the direction
            # text variables
            dX = pts[-10][0] - pts[i][0]
            dY = pts[-10][1] - pts[i][1]
            (dirX, dirY) = ("", "")
 
            # ensure there is significant movement in the
            # x-direction
            if np.abs(dX) > 20:
                dirX = "East" if np.sign(dX) == 1 else "West"
 
            # ensure there is significant movement in the
            # y-direction
            if np.abs(dY) > 20:
                dirY = "North" if np.sign(dY) == 1 else "South"
 
            # handle when both directions are non-empty
            if dirX != "" and dirY != "":
                direction = "{}-{}".format(dirY, dirX)
 
            # otherwise, only one direction is non-empty
            else:
                direction = dirX if dirX != "" else dirY
        # otherwise, compute the thickness of the line and
        # draw the connecting lines
        thickness = int(np.sqrt(32 / float(i + 1)) * 2.5)
        cv2.line(frame, pts[i - 1], pts[i], (0, 0, 255), thickness)
 
    # show the movement deltas and the direction of movement on
    # the frame
    cv2.putText(frame, direction, (10, 30), cv2.FONT_HERSHEY_SIMPLEX,
        0.65, (0, 0, 255), 3)
    cv2.putText(frame, "dx: {}, dy: {}".format(dX, dY),
        (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX,
        0.35, (0, 0, 255), 1)
 
    # show the frame to our screen and increment the frame counter
    cv2.imshow("Frame", frame)
    res = cv2.bitwise_and(frame, frame, mask=mask)
    cv2.imshow('res',res)
    key = cv2.waitKey(1) & 0xFF
    counter += 1
 
    # if the 'q' key is pressed, stop the loop
    if key == ord("q"):
        break
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()