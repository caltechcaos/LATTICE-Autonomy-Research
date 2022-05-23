import networkx as nx
import pickle
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image
import time

START = (200, 1600)
END =(3000, 1600)

G = nx.read_gpickle('0-3200_1598-1602.gpickle')

def dist(a, b):
    (x1, y1) = a
    (x2, y2) = b
    return ((x1 - x2) ** 2 + (y1 - y2) ** 2) ** 0.5

startTime = time.time() 
wow = nx.shortest_path(G, source=START, target=END, method='dijkstra')
# wow = nx.astar_path(G, source=START, target=END, heuristic=dist)
executionTime = (time.time() - startTime) # For timing
print('Execution time in seconds: ' + str(executionTime))
print(len(wow))

fig = plt.figure()
ax = fig.add_subplot(111)

DATA_STR = 'D:\Dropbox\Projects\LATTICE\LATTICE_Autonomy\Site04_final_adj_5mpp_surf.tif'
MAP = np.array(Image.open(DATA_STR))
ORIGIN = 'lower'
POINT_COL = 'black'
CMAP = 'gist_heat'
LINE_COL = 'white' #'#B22222'
plt.imshow(MAP, cmap=CMAP, origin=ORIGIN)
plt.colorbar()

def draw_point(point):
	plt.scatter(point[0], point[1], c=POINT_COL, linewidths=1, edgecolors='white')

draw_point(START)
draw_point(END)

plt.plot(*zip(*wow), 'w')
plt.show()