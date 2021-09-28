# x = cos(theta)*cos(phi)*r
# y = sin(theta) * cos(phi) * r
# z = sin(phi) * r
# theta is the pan angle, phi is the tilt angle, r is the sensor value for distance

from math import sin, cos
import numpy as np

#do the math
points = []

def dataToDistance(data):
    #data is sensor reading
    distance = (data-644)/-19.9
    return distance

def coordinateConvert(theta, phi, r):
    #r is distance measured by sensor in inches
    #theta is pan angle
    #phi is tilt angle
    
    x_coord = cos(theta)*cos(phi)*r
    y_coord = sin(theta) * cos(phi) * r
    z_coord = sin(phi) * r
    return [x_coord, y_coord, z_coord]


#do the plotting
#create array with 3 rows, x, y, z

a= [[1, 2, 2, 1, 2,4 ,3], [2, 3, 4, 2, 3, 2, 4], [1, 2, 3, 4, 5, 6, 7]]
data = np.array(a)
print(data.shape)
x = data[:, 0]
y = data[:, 1]
z = data[:, 2]

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import proj3d

fig = plt.figure(figsize=(8, 8))
ax = fig.add_subplot(111, projection='3d')

ax.scatter(x, y, z)
plt.show()

#take off or redefine/change colours of points in plot