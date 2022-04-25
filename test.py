import numpy as np
import scipy.interpolate as interp
import matplotlib.pyplot as plt
from PIL import Image

DATA_STR = 'Site04_final_adj_5mpp_surf.tif'
ORIGIN = 'lower'
START_END_COL = 'black'
POINT_COL = 'red'

CABLE_LEN = 20 # length of the cable in pixels
COUNT = 20 # number of points to sample in each circle

HEIGHT = 0.5

START = np.array([200, 1600]) # [x y]
END = np.array([200, 3000])

# loads and parses the data
def get_data(data_str):
	data_img = Image.open(data_str)
	return np.array(data_img)

# draws the entire space, including the start and end
def draw_data(data):
	plt.imshow(data, cmap='gray', origin=ORIGIN)
	plt.colorbar()

	plt.scatter(START[0], START[1], c=START_END_COL)
	plt.scatter(END[0], END[1], c=START_END_COL)

# draws a small area around the start
def draw_start(data, sec):
	# center
	start_x = START[0]
	start_y = START[1]

	# edges of the section to draw
	edges = [start_x - sec, start_x + sec, start_y - sec, start_y + sec]

	plt.imshow(data[edges[0]:edges[1], edges[2]:edges[3]], 
		extent=edges, cmap='gray', origin=ORIGIN)
	plt.colorbar()
	plt.scatter(start_x, start_y, c=START_END_COL)

# returns the interpolated function of the data
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
	safe = np.nonzero(errs == 0)[0]

	sc = plt.scatter(x, y, c=diffs, linewidths=errs_all.flatten(), edgecolors='black')
	plt.colorbar(sc)

	return safe

if __name__ == "__main__":
	data = get_data(DATA_STR)

	interp = interpolate(data)
	safe = circ_check(interp, START)
	print(safe)

	draw_start(data, 30)
	# draw_data(data)
	plt.show()