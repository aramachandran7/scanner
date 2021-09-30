#      ******************************************************************
#      *                                                                *
#      *                                                                *
#      *    Example Python program that receives data from an Arduino   *
#      *                                                                *
#      *                                                                *
#      ******************************************************************

#get data imports
import serial
import keyboard

#data to coord imports
from math import sin, cos, radians
import numpy as np
from StoreAndPlotDummy import dataToDistance

#plotting imports
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import proj3d

#
# Note 1: This python script was designed to run with Python 3.
#
# Note 2: The script uses "pyserial" which must be installed.  If you have
#         previously installed the "serial" package, it must be uninstalled
#         first.
#
# Note 3: While this script is running you can not re-program the Arduino.
#         Before downloading a new Arduino sketch, you must exit this
#         script first.
#


#
# Set the name of the serial port.  Determine the name as follows:
#	1) From Arduino's "Tools" menu, select "Port"
#	2) It will show you which Port is used to connect to the Arduino
#
# For Windows computers, the name is formatted like: "COM6"
# For Apple computers, the name is formatted like: "/dev/tty.usbmodemfa141"
#
arduinoComPort = "/dev/ttyUSB0"


#
# Set the baud rate
# NOTE1: The baudRate for the sending and receiving programs must be the same!
# NOTE2: For faster communication, set the baudRate to 115200 below
#        and check that the arduino sketch you are using is updated as well.
#
baudRate = 115200


#
# open the serial port
#
serialPort = serial.Serial(arduinoComPort, baudRate, timeout=1)

#defining lists for data storage
x = []
y = []
z = []


# main loop to read data from the Arduino, then store & display it
while True:
    #
    # ask for a line of data from the serial port, the ".decode()" converts the
    # data from an "array of bytes", to a string
    #
    lineOfData = serialPort.readline().decode()
    #decode()

    #
    # check if data was received
    #

    # distance_measurement = int(lineOfData)
    # print(distance_measurement)  # if user pressed a key other than the given key the loop will continue

    if len(lineOfData) > 8:
    #
    # data was received, convert it into 4 integers
        #print(lineOfData)
        a = lineOfData.split(",")
        print(a)

        # #a0=pan angle in degrees, a1=tilt angle in deg, a2=sensor data

        # #turn data into coordinates & add to final plotting lists
        x.append(cos(radians(int(a[0])))*cos(radians(int(a[1])*dataToDistance(int(a[2])))))
        y.append(sin(radians(int(a[0])))*cos(radians(int(a[1])))*dataToDistance(int(a[2])))
        z.append(sin(radians(int(a[1])))*dataToDistance(int(a[2])))
    
    # once line of data = 3 (in other words all data is collected), plot
    if len(lineOfData)==3:
        break
    else:
        continue

fig = plt.figure(figsize=(10, 10))

ax = fig.add_subplot(111, projection='3d')

ax.scatter(x, y, z)
plt.show()
