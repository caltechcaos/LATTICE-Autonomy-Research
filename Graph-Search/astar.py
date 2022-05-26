import networkx as nx
import pickle
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image
import time

START = (210, 1600)
END =(2950, 1600)

G = nx.read_gpickle('0-3199_1598-1602_100.0_0.05.gpickle')

def dist(a, b):
    (x1, y1) = a
    (x2, y2) = b
    return ((x1 - x2) ** 2 + (y1 - y2) ** 2) ** 0.5

startTime = time.time() 
path = nx.shortest_path(G, source=START, target=END, method='dijkstra')
# path = nx.astar_path(G, source=START, target=END, heuristic=dist)
executionTime = (time.time() - startTime) # For timing
print('Execution time in seconds: ' + str(executionTime))
print(len(path))

fig = plt.figure()
ax = fig.add_subplot(111)

DATA_STR = 'D:\Dropbox\Projects\LATTICE\LATTICE_Autonomy\Site04_final_adj_5mpp_surf.tif'
MAP = np.array(Image.open(DATA_STR))
MAP_W, MAP_H = MAP.shape    # Width and Height of map   (pixels)
pix2m = 5                   # Conversion factor         (m/pixel)
ORIGIN = 'lower'
POINT_COL = 'black'
CMAP = 'gnuplot'
LINE_COL = 'white' #'#B22222'
plt.imshow(MAP, cmap=CMAP, origin=ORIGIN, extent=[0, pix2m*MAP_W, 0, pix2m*MAP_H])
cbar = plt.colorbar()

def draw_point(point):
	plt.scatter(point[0]*pix2m, point[1]*pix2m, c=POINT_COL, linewidths=1, edgecolors='white')

draw_point(START)
draw_point(END)

path = [(x*pix2m, y*pix2m) for x, y in path]
plt.plot(*zip(*path), 'w')
plt.title('500 m Cable Length, 5 cm Ground Clearance')
plt.xlabel('x (m)')
plt.ylabel('y (m)')
cbar.set_label('Elevation (m)')
plt.show()