#!/usr/bin/env python
# coding=utf-8

# This turns off the HV tube supply

import smbus

i2c_addr = 0x19

# I2C config for HV supply
bus = smbus.SMBus(1)  # I2C1 port
bus.write_byte(i2c_addr, 0)  # set pulse-width to zero (off)
