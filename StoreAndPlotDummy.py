"""
Document used for testing data storage + plotting with dummy data
"""


# x = cos(theta)*cos(phi)*r
# y = sin(theta) * cos(phi) * r
# z = sin(phi) * r
# theta is the pan angle, phi is the tilt angle, r is the sensor value for distance

from math import sin, cos, radians
import numpy as np

#do the math
points = []

def dataToDistance(data):
    #data is sensor reading
    distance = (data-644)/-19.9
    return distance

def coordinateConvert(pan, tilt, r):
    #r is distance measured by sensor in inches
    #theta is pan angle
    #phi is tilt angle
    
    x_coord = cos(pan)*cos(tilt)*r
    y_coord = sin(pan) * cos(tilt) * r
    z_coord = sin(tilt) * r
    return [x_coord, y_coord, z_coord]

x = []
y=[]
z=[]
#do the plotting
#create array with 3 rows, x, y, z

#turn line of data into list
lineOfData = "45, 45, 300"
a = lineOfData.split(",")
#a0=pan angle in degrees, a1=tilt angle in deg, a2=sensor data

#turn data into coordinates & add to final plotting lists
x.append(cos(radians(int(a[0]))*cos(radians(int(a[1])*dataToDistance(int(a[2]))))))
y.append(sin(radians(int(a[0])))*cos(radians(int(a[1])))*dataToDistance(int(a[2])))
z.append(sin(radians(int(a[1])))*dataToDistance(int(a[0])))

#loop through data for plotting
# data = np.array(a)
# print(data.shape)
# x = cos(pan[i])*cos(tilt[i])*r
# y = data[:, 1]
# z = data[:, 2]


#PLOTTING
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import proj3d

fig = plt.figure(figsize=(8, 8))
ax = fig.add_subplot(111, projection='3d')

ax.scatter(x, y, z)
plt.show()

#take off or redefine/change colours of points in plot