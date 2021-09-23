#!/usr/local/bin/python

from serial import Serial, SerialException

# The Serial constructor will take a different first argument on
# Windows. The first argument on Windows will likely be of the form
# 'COMX' where 'X' is a number like 3,4,5 etc.
# Eg.cxn = Serial('COM5', baudrate=9600
#
# NOTE: You won't be able to program your Arduino or run the Serial
# Monitor while the Python script is running.


# START - 1
# STOP - 0

cxn = Serial('/dev/tty.usbmodem1411', baudrate=9600)

while(True):
    try:
        cmd_id = int(raw_input("Please enter a command ID (1 - read potentiometer, 2 - read the button: "))
        if int(cmd_id) > 2 or int(cmd_id) < 1:
            print "Values other than 1 or 2 are ignored."
        else:
            cxn.write([int(cmd_id)])
            while cxn.inWaiting() < 1:
                pass
            result = cxn.readline();
            if result>0:
                break
            else:
                # FAULT
                pass
            print result
    except ValueError:
        print "You must enter an integer value between 1 and 2."

# RECIEVE
