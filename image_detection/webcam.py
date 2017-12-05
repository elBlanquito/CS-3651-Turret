#! /usr/bin env python

import numpy as np
import cv2
import serial
import imutils
from scipy.stats import itemfreq
from time import sleep
import os

def acquire_conn():
    ser = None
    num = 0
    while True:
        try:
            ser = serial.Serial(f'/dev/ttyUSB{num}', 115200)
            print('Established Link with Arduino')
            break
        except:
            print('Waiting on Arduino')
            if num == 0:
                num = 1
            else:
                num = 0
            sleep(1)
            continue
    return ser

def acquire_target(cap):
    ret, frame = cap.read()
    while not ret:
        cap = cv2.VideoCapture(0)
        ret, frame = cap.read()
    frame = imutils.resize(frame, width=512, height=512)
    height, width = frame.shape[:2]
    img = frame[int(height/2 - 50):int(height/2 + 50), int(width/2 - 50):int(width/2 + 50)]
    arr = np.float32(img)
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
    print(lower_mask, upper_mask)
    return lower_mask, upper_mask, cap

def get_cords(lower_mask, upper_mask, cap, ser):
    coords = []
    for i in range(5):
        ret, frame = cap.read()
        while not ret:
            cap2 = cv2.VideoCapture(0)
            ret, frame = cap2.read()
        frame = imutils.resize(frame, width=256, height=256)
        blurred = cv2.GaussianBlur(frame, (11, 11), 0)
        hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
        mask = cv2.inRange(hsv, lower_mask, upper_mask)
        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)
        cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
            cv2.CHAIN_APPROX_SIMPLE)[-2]
        center = None
        if len(cnts) > 0:
            c = max(cnts, key=cv2.contourArea)
            ((x, y), radius) = cv2.minEnclosingCircle(c)
            M = cv2.moments(c)
            center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

            if radius > 10:
                coords.append(center)
    if len(coords) == 5:
        xs, ys = zip(*coords)
        x, y = int(np.average(xs) * 4), int(np.average(ys) * 5.24)
        if 350 <= x <= 600:
            x = 513
        if 350 <= y <= 600:
            y = 500
        try:
            ser.write(f'{x},{y}'.encode())
        except:
            acquire_conn()

lower_mask, upper_mask = None, None
ser = acquire_conn()
cap = None
while True:
    try:
        if ser.inWaiting() > 0:
            line = ser.readline().decode().strip()
            if line == 'auto':
                print('Auto Mode')
                cap = cv2.VideoCapture(0)
                lower_mask, upper_mask, cap = acquire_target(cap)
            elif line == 'manual':
                print('Manual Mode')
                lower_mask, upper_mask, cap = None, None, None
        else:
            if lower_mask is not None:
                get_cords(lower_mask, upper_mask, cap, ser)
        sleep(.1)
    except OSError:
        ser = acquire_conn()

