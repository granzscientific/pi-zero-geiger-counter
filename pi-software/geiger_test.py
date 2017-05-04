#!/usr/bin/env python
# coding=utf-8

import sys

import RPi.GPIO as GPIO
import smbus

try:
    input = raw_input  # Python2
except NameError:
    pass  # Python3

i2c_addr = 0x19
gpio_pin = 7
total_count = 0

# I2C config for HV
bus = smbus.SMBus(1)  # I2C1 port
bus.write_byte(i2c_addr, 0x71)

# GPIO.setmode(GPIO.BCM)
# GPIO.setup(gpio_pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(gpio_pin, GPIO.IN)


def my_callback(channel):
    global total_count
    total_count = total_count + 1
    sys.stdout.write('*')
    sys.stdout.flush()


GPIO.add_event_detect(gpio_pin, GPIO.FALLING)
GPIO.add_event_callback(gpio_pin, my_callback)

while True:
    input('')
    print(str(total_count))
    # test = GPIO.input(gpio_pin)
    # print 'Got: ' + str(test)
