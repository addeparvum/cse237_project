from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import sys
import imutils
import numpy as np


def filter_fn(img):

	cascPath = 'haarcascade_frontalface_default.xml'

	gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    '''
        :param img: A numpy array representing the input image
        :returns: A numpy array to send to the mjpg-streamer output plugin
    '''
    return img
    
def init_filter():
    return filter_fn