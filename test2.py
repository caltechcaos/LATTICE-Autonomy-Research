from operator import index, length_hint
from pickle import FALSE
import numpy as np
import scipy.interpolate as interp
import matplotlib.pyplot as plt
from PIL import Image
import random

from sklearn import tree

DATA_STR = 'Site04_final_adj_5mpp_surf.tif'
ORIGIN = 'lower'
POINT_COL = 'black'

CABLE_LEN = 20 #20 original length of the cable in pixels
COUNT = 40 # number of points to sample in each circle
ZOOM = 30 # how many pixels to draw on each side when zoomed in
SHOW_ALL = False # whether to draw the whole space, or a specific point

HEIGHT = .5 # height cables are allowed off ground

START = np.array([200, 1600]) # [x y]
END = np.array([3000, 1600])

# loads and parses the data
def get_data(data_str):
	data_img = Image.open(data_str)
	return np.array(data_img)


# draws the entire space, including the start and end
def draw_data(data):
	plt.imshow(data, cmap='terrain', origin=ORIGIN)
	plt.colorbar()

	draw_point(START)
	draw_point(END)

# draws a single point
def draw_point(point):
	plt.scatter(point[0], point[1], c=POINT_COL, linewidths=.25)

def draw_line(point1, point2):
    plt.plot([point1[0], point2[0]],[point1[1], point2[1]])



def interpolate(data):
	shape = data.shape
	x = np.arange(shape[0]) # [0 ... x_max]
	y = np.arange(shape[1])
	return interp.RectBivariateSpline(x, y, data)

# generates a circularly expanding set of sampling points around the given center
def circ_sample(rad, num, center):
	# list of possible angles and radii
	# starts to the right, heads counter-clockwise
	angles = np.linspace(0, 2 * np.pi, num, endpoint=False) # [0 ... 2 pi] (num)
	radii = np.arange(0, rad + 1) # [1 ... rad]
	
	# coordinates of a circle of radius 1
	x = np.cos(angles) # (num)
	y = np.sin(angles)

	# multiply those coordinates by each possible radius
	xs = np.outer(x, radii) + center[0] # (num, radii)
	ys = np.outer(y, radii) + center[1]

	return xs, ys

# interpolates in a circle around the given center
# returns the directions which are valid
def circ_check(interp, center):
	# points to interpolate at
	x, y = circ_sample(CABLE_LEN, COUNT, center)

	# interpolated data
	approx = interp(x, y, grid=False) # (num, radii)

	# calculate slopes
	num = approx.shape[1]
	starts = approx[:, 0] + HEIGHT
	ends = approx[:, num - 1] + HEIGHT
	slopes = np.linspace(starts, ends, num, axis=-1)

	# apply slopes and find errors
	diffs = slopes - approx
	errs_all = diffs < 0
	errs = np.sum(errs_all, -1)

	# sc = plt.scatter(x, y, c=diffs, linewidths=errs_all.flatten(), edgecolors='black')
	# plt.colorbar(sc)

	# find points which are safe to pick up from
	safe = np.nonzero(errs == 0)[0]
	safe_x = x[safe, num - 1]
	safe_y = y[safe, num - 1]

	return safe_x, safe_y


# draws a small area around the start
def draw_zoom(data, center, sec):
	center_x = center[0]
	center_y = center[1]

	# edges of the section to draw
	edges = [center_x - sec, center_x + sec, center_y - sec, center_y + sec]

	plt.imshow(data[edges[0]:edges[1], edges[2]:edges[3]], 
		extent=edges, cmap='terrain', origin=ORIGIN)
	plt.colorbar()
	draw_point(center)


class Node:
   def __init__(self, data, parent):
      self.children = []
      self.parent = parent
      self.data = data





data = get_data(DATA_STR)
interp2d = interpolate(data)
# shape = data.shape
# x = np.arange(shape[0]) # [0 ... x_max]
# y = np.arange(shape[1])
# slopeX = interp2d(x, y, dx = 2, dy = 0, grid = FALSE)
# slopeY = interp2d(x, y, dx = 0, dy = 1, grid = FALSE)
# norm = np.power(slopeX, 2)+ np.power(slopeY, 2)
# # draw_data(data)

# # try to see the slope
# # print(np.shape(data2))
# print(np.max(slopeX))
# print(np.min(slopeX))
# print(np.average(slopeX))

# lets true to generation some sort of random tree, very basic

#start with a certain number of trees
t_num = 5
g_num = 500
max_points = 30


# generate initial trees, randomely select from valid points which get us closer to the goal point

safe_x, safe_y = circ_check(interp2d, START)
# print(np.shape(safe_x))
# print(START[0])
# print((safe_x - START[1])>0)
safe_y = safe_y[(safe_x - START[0])>0]
safe_x = safe_x[(safe_x - START[0])>0]
pos_index = random.sample(range(0, len(safe_y)), t_num)
safe_y = safe_y[pos_index]
safe_x = safe_x[pos_index]

stop = False
cur_points = []
# plot safe points test
for i in range(0, len(safe_x)):
    # draw_point([safe_x[i],safe_y[i]])
    # draw_line(START, [safe_x[i],safe_y[i]])
    cur_points.append([safe_x[i],safe_y[i]])

start = Node(START, None)
for i in range(0, len(cur_points)):
    start.children.append(Node(cur_points[i],start))
    # start.children[i].parent = start


cur_nodes = start.children
cur_points = np.array(cur_points)
for i in range(0, g_num):

    next_points = []
    next_nodes = []
    # pick only the best 20 points
    if(len(cur_points)>max_points):
        dist = []
        for j in range(0, len(cur_points)):
            dist.append(np.linalg.norm(END - np.array(cur_points[j])))

        s = np.argsort(dist)
        if(dist[s[0]] < 50):
            break
        # print(cur_points)
        # print(s)
        cur_points = cur_points[s]
        cur_points = cur_points[0:max_points]
        
        #sort list of nodes
        new_nodes = []
        count = 0
        for j in s:
            new_nodes.append(cur_nodes[j])
            count = count +1
        cur_nodes = new_nodes[0:max_points]

        #print(len(cur_points))

    for j in range(0, len(cur_points)):
        safe_x, safe_y = circ_check(interp2d, cur_points[j])
        # print(np.shape(safe_x))
        # print(START[0])
        # print((safe_x - START[1])>0)

        # safe_y = safe_y[(safe_x - cur_points[j][0])>0]
        # safe_x = safe_x[(safe_x -  cur_points[j][0])>0]
        # indexs = np.abs((safe_x - cur_points[j][0]))>np.abs((safe_y- cur_points[j][1]))


        indexs = (safe_x - cur_points[j][0])>10
        safe_y = safe_y[indexs]
        safe_x = safe_x[indexs]
        
        if(len(safe_y) ==0):
            continue
        elif(len(safe_y) < t_num):
            pos_index = random.sample(range(0, len(safe_y)), len(safe_y))
        else:
            pos_index = random.sample(range(0, len(safe_y)),t_num)
        
        safe_y = safe_y[pos_index]
        safe_x = safe_x[pos_index]

        for ii in range(0, len(safe_x)):

            # draw_point([safe_x[ii],safe_y[ii]])
            # draw_line(cur_points[j], [safe_x[ii],safe_y[ii]])
            next_points.append([safe_x[ii],safe_y[ii]])
            next_nodes.append(Node([safe_x[ii],safe_y[ii]],cur_nodes[j]))
            # next_nodes[ii].parent = cur_nodes[j]

    if(len(next_points)!= 0):
        cur_points = np.array(next_points)
        cur_nodes = next_nodes

#now plot using our tree
# print(cur_points)
# print(cur_nodes)
# print(cur_points)
end = cur_nodes[0]
count =0

while(not end.parent == None):
    draw_point(end.data)
    draw_line(end.data, end.parent.data)
    end = end.parent
    count = count + 1
print(count)


# for i in range(0,len(cur_nodes)):
#     print(cur_points[i], cur_nodes[i].data)
    

# draw_zoom(data,np.array([600,1600]), 500)
draw_data(data)
plt.show()