from turtle import color
import numpy as np
import scipy.interpolate as interp
import matplotlib.pyplot as plt
from PIL import Image

from kdTree import kdTree
from kdTree import node
import random, time, math
from math import sqrt,cos,sin,atan2,pi

DATA_STR = 'D:\Dropbox\Projects\LATTICE\LATTICE_Autonomy\Site04_final_adj_5mpp_surf.tif'
MAP = np.array(Image.open(DATA_STR))
# returns the interpolated function of the data
def interpolate(data):
	shape = data.shape
	x = np.arange(shape[0]) # [0 ... x_max]
	y = np.arange(shape[1])
	return interp.RectBivariateSpline(x, y, data)
INTERP = interpolate(MAP)
MAP_W, MAP_H = MAP.shape # Width and Height of map (pixels)

ORIGIN = 'lower'
POINT_COL = 'black'
CMAP = 'gnuplot'
LINE_COL = 'white' #'#B22222'

CABLE_LEN = 300/5 # length of the cable in pixels
COUNT = 20 # number of points to sample in each circle
ZOOM = 30 # how many pixels to draw on each side when zoomed in
SHOW_ALL = False # whether to draw the whole space, or a specific point

HEIGHT = 0.5 # height cables are allowed off ground

END = np.array([200, 1600]) # [x y]
START = np.array([3000, 1600])

BIG_NUMBER = 100000000000000

# draws the entire space, including the start and end
def draw_data(data):
	plt.imshow(data, cmap=CMAP, origin=ORIGIN)
	plt.colorbar()

	draw_point(START)
	draw_point(END)

# draws a small area around the start
def draw_zoom(data, center, sec):
	center_x = center[0]
	center_y = center[1]

	# edges of the section to draw
	edges = [center_x - sec, center_x + sec, center_y - sec, center_y + sec]

	plt.imshow(data[edges[0]:edges[1], edges[2]:edges[3]], 
		extent=edges, cmap='gray', origin=ORIGIN)
	plt.colorbar()
	draw_point(center)

# draws a single point
def draw_point(point):
	plt.scatter(point[0], point[1], c=POINT_COL, linewidths=1, edgecolors='white')

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

def line_to_gridpts(p1, p2):
	# Find the difference to do all calc wrt origin
	pt = p2 - p1

	# Check whether |y| > |x|
	if abs(pt[1]) > abs(pt[0]):
		ratio = pt[0] / pt[1]
		param = np.arange(0, int(np.ceil(np.abs(pt[1]))), 1, np.sign(pt[1]))[:,None]
		high = np.ceil(ratio*param)
		low = np.floor(ratio*param)
		points = np.vstack( (np.hstack((high, param)) , np.hstack((low, param))) )
	else:
		ratio = pt[1] / pt[0]
		param = np.arange(0, int(np.ceil(np.abs(pt[0]))), 1, np.sign(pt[1]))[:,None]
		high = np.ceil(ratio*param)
		low = np.floor(ratio*param)
		points = np.vstack( (np.hstack((param, high)) , np.hstack((param, low))) )
	
	return points + p1

def check_validity(A, B):
	# Returns True if stake placement is valid
	points = line_to_gridpts(A,B)

	# Add z values corresponding to points
	z_vals = MAP[points[:,0].astype(int), points[:,1].astype(int)]
	points = np.hstack((points, z_vals[:,None]))

	# TODO check this next move
	A = np.array([A[0], A[1], z_vals[0]])
	B = np.array([B[0], B[1], z_vals[-1]])

	v = B-A
	perp_v = np.array([-v[1], v[0], B[2]])
	orth_v = np.cross(v, perp_v)
	orth_v = orth_v / np.linalg.norm(orth_v)

	a2points = points - A
	projection = np.dot(a2points, orth_v)

	return all(projection <= HEIGHT)

def get_distance_and_angle(node_start, node_end):
	dx = node_end[0] - node_start[0]
	dy = node_end[1] - node_start[1]
	return math.hypot(dx, dy), math.atan2(dy, dx)

def new_state(node_start, node_end):
	dist, theta = get_distance_and_angle(node_start, node_end)

	dist = min(CABLE_LEN, dist)
	node_new = np.array([node_start[0] + dist * math.cos(theta),
						node_start[1] + dist * math.sin(theta)])

	if not check_validity(node_start, node_new):
		return np.array([-1,-1])

	return node_new

PERCENT_GOAL = 0.1
dim = 2

plt.clf()
plt.gcf().canvas.mpl_connect('key_release_event',
lambda event: [exit(0) if event.key == 'escape' else None])
plt.xlim(MAP_W)
plt.ylim(MAP_H)
draw_data(MAP)

class RRTAlgorithm(object):
	def __init__(self, source, goal): #initial and destination coordinates
		print(source, goal)
		self.start(source, goal)
		
	def start(self, source, goal):
		
		RRTree = node(source, [], None, True) #actual RRTree
		Points = kdTree(None, None, 0, source, RRTree) #for storing generated points to increase the search complexity
		current = source
		
		while not self.check(current, goal):
			# Sample a point randomly to grow the tree towards
			if random.random() <= PERCENT_GOAL:
				rand = goal
			else:
				# rand = [random.random() * MAP_W, random.random() * MAP_H]
				rand = self.gen_rand_coords(Points.dist(goal)/3.0)
			
			# rand = self.gen_rand_coords(Points.dist(goal)/3.0)
			
			retNN = Points.search(rand, BIG_NUMBER, None, None, None, None, None)
			nearest_neighbour = retNN[1]
			new_point = new_state(nearest_neighbour, rand)
			# print(new_point)
			if np.array_equal(new_point, np.array([-1,-1])):
				continue
			# time.sleep(0.1)
			nde = node(new_point, [], retNN[2], True)
			retNN[2].add_child(nde)
			Points.insert(new_point, dim, nde)
			current = new_point

			plt.plot([nearest_neighbour[0], new_point[0]],
			[nearest_neighbour[1], new_point[1]], color="gray")
			plt.pause(0.001)

		retNN = Points.search(current, BIG_NUMBER, None, None, None, None, None)
		nde = retNN[2]

		while nde.parent != None:
			plt.plot([nde.point[0], nde.parent.point[0]],[nde.point[1], nde.parent.point[1]], color=LINE_COL)
			nde = nde.parent
			# plt.pause(0.01)

	def gen_rand_coords(self, sigma):

		x = 100000000
		y = 100000000
		sigma_radius = sigma
		while(x > MAP_W or y > MAP_H or x < 0 or y < 0):

			r = np.random.normal(0, sigma_radius, 1)
			r = r[0]
			theta = random.random()*2*pi
			x = cos(theta)*r + END[0]
			y = sin(theta)*r + END[1]
		return x,y

	def check(self, point , goal): # checking if currently added node is at goal or not
		tol = 50 #tolerance distance from goal
		if point[0] > goal[0]-tol and point[0] < goal[0]+tol and point[1] > goal[1]-tol and point[1] < goal[1]+tol:
			return True
		return False

	def dist(self, p1, p2): #returns euclid's distance between points p1 and p2
		return sqrt((p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1]))

	# def step_from_to(self, p1, p2): #returns point with cable_len distance from nearest neighbour in the direction of randomly generated point
	# 	theta = atan2(p2[1] - p1[1], p2[0] - p1[0])
	# 	return [p1[0] + CABLE_LEN * cos(theta), p1[1] + CABLE_LEN * sin(theta)]

	def circ_step(self, center, point):
		# points to interpolate at
		x, y = circ_sample(CABLE_LEN, COUNT, center)

		# interpolated data
		approx = INTERP(x, y, grid=False) # (num, radii)

		# calculate slopes
		num = approx.shape[1]
		starts = approx[:, 0] + HEIGHT
		ends = approx[:, num - 1] + HEIGHT
		slopes = np.linspace(starts, ends, num, axis=-1)

		# apply slopes and find errors
		diffs = slopes - approx
		errs_all = diffs < 0
		errs = np.sum(errs_all, -1)

		# find points which are safe to pick up from
		safe = np.nonzero(errs == 0)[0]
		safe_x = x[safe, num - 1]
		safe_y = y[safe, num - 1]
		
		idx = closest_node(point, np.hstack((safe_x[:,None], safe_y[:,None])))

		return [safe_x[idx], safe_y[idx]]

def closest_node(node, nodes):
	dist_2 = np.sum((nodes - node)**2, axis=1)
	return np.argmin(dist_2)

if __name__ == "__main__":
	tree = RRTAlgorithm(START, END)
	plt.show()

# 	if SHOW_ALL:
# 		draw_data(data)
# 	else:
# 		point = START

# 		interp = interpolate(data)
# 		safe_x, safe_y = circ_check(interp, point)
# 		print(safe_x, safe_y)

# 		draw_zoom(data, point, ZOOM)

# 	plt.show()