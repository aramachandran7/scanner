#      ******************************************************************
#      *                                                                *
#      *                                                                *
#      *    Example Python program that receives data from an Arduino   *
#      *                                                                *
#      *                                                                *
#      ******************************************************************


import serial
import keyboard
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
arduinoComPort = "/dev/ttyACM0"


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



#
# main loop to read data from the Arduino, then display it
#
while True:
    try:  # used try so that if user pressed other than the given key error will not be shown
        if keyboard.is_pressed('q'):  # if key 'q' is pressed
            print('You Pressed A Key!')
            # break  # finishing the loop
    except:
        #
        # ask for a line of data from the serial port, the ".decode()" converts the
        # data from an "array of bytes", to a string
        #
        lineOfData = serialPort.readline().decode()

        #
        # check if data was received
        #

        # distance_measurement = int(lineOfData)
        # print(distance_measurement)  # if user pressed a key other than the given key the loop will continue


    if len(lineOfData) > 0:
    #
    # data was received, convert it into 4 integers
    #
        print(lineOfData)
    # therm_a, therm_b = (int(x) for x in lineOfData.split(','))
    #
    # #
    # # print the results
    # #
    # print("a = " + str(a), end="")
    # print(", b = " + str(b), end="")
    # print(", c = " + str(c), end="")
    # print(", d = " + str(d))
