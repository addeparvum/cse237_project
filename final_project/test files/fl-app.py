#!/usr/bin/python

from flask import request
from flask_api import FlaskAPI
import RPi.GPIO as GPIO

LEDS = {"green": 10, "red": 12}
GPIO.setmode(GPIO.BCM)
GPIO.setup(LEDS["green"], GPIO.OUT)
GPIO.setup(LEDS["red"], GPIO.OUT)

app = FlaskAPI(__name__)

@app.route('/', methods=["GET"])
def api_root():
    return {
           "led_url": request.url + "led/(green | red)/",
      		 "led_url_POST": {"state": "(0 | 1)"}
    			 }
  
@app.route('/led/<color>/', methods=["GET", "POST"])
def pi_leds_control(color):
    print(request.method)
    if request.method == "POST":
        print("POST REQUEST")
        GPIO.output(10,int(request.data.get("state")))
    return {color: GPIO.input(LEDS[color])}

if __name__ == "__main__":
    app.run()
