import numpy as np
import cv2
import serial
import imutils
from scipy.stats import itemfreq
from time import sleep
import os

def acquire_target():
    # Capture frame-by-frame
    cap = cv2.VideoCapture(0)
    ret, frame = cap.read()
    while not ret:
        cap = cv2.VideoCapture(0)
        ret, frame = cap.read()
    frame = imutils.resize(frame, width=512, height=512)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Dominant color of center, using kmeans
    # https://stackoverflow.com/questions/43111029/how-to-find-the-average-colour-of-an-image-in-python-with-opencv
    arr = np.float32(frame)
    pixels = arr.reshape((-1, 3))

    n_colors = 5
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 200, .1)
    flags = cv2.KMEANS_RANDOM_CENTERS
    _, labels, centroids = cv2.kmeans(pixels, n_colors, None, criteria, 10, flags)

    palette = np.uint8(centroids)
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
    return lower_mask, upper_mask

def get_cords(lower_mask, upper_mask, ser):
    cap = cv2.VideoCapture(0)
    ret, frame = cap.read()
    while not ret:
        cap = cv2.VideoCapture(0)
        ret, frame = cap.read()
    frame = imutils.resize(frame, width=512, height=512)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    # https://www.pyimagesearch.com/2015/09/21/opencv-track-object-movement/
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, lower_mask, upper_mask)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)
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
            ser.write('{0},{1}'.format(*center).encode())

def acquire_conn():
    ser = None
    while True:
        try:
            ser = serial.Serial('/dev/ttyUSB0', 115200)
            print('Established Link with Arduino')
            break
        except:
            print('Waiting on Arduino')
            sleep(1)
            continue
    return ser

if __name__ == '__main__':
    lower_mask, upper_mask = None, None
    ser = acquire_conn()
    while True:
        if ser.inWaiting() > 0:
            line = ser.readline().decode().strip()
            if line == 'auto':
                print('Auto Mode')
                lower_mask, upper_mask = acquire_target()
            elif line == 'manual':
                print('Manual Mode')
                lower_mask, upper_mask = None, None
            ser.flushInput()
            ser.flushOutput()
        else:
            if lower_mask:
                get_cords(lower_mask, upper_mask, ser)
        sleep(.1)

