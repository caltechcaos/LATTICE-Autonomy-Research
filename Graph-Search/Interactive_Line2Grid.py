import mpl_interactions.ipyplot as iplt
import matplotlib.pyplot as plt
import numpy as np
from skimage.draw import line

# For the underlying grid
grid_size = 50
# create one-dimensional arrays for x and y
x = np.arange(grid_size+1)
y = np.arange(grid_size+1)
# create the mesh based on these arrays

X, Y = np.meshgrid(x, y)

# For the circle
neighbors = 50
theta = np.linspace(0, 2 * np.pi , 50)
radius = 20

def linx(x1, y1, x2, y2):
    #assert , 'Out of range!'
    x1 = int(np.round(x1))
    y1 = int(np.round(y1))
    x2 = int(np.round(x2))
    y2 = int(np.round(y2))
    rr, cc = line(x1, y1, x2, y2)
    return rr

def liny(x, x1, y1, x2, y2):
    #assert , 'Out of range!'
    x1 = int(np.round(x1))
    y1 = int(np.round(y1))
    x2 = int(np.round(x2))
    y2 = int(np.round(y2))
    rr, cc = line(x1, y1, x2, y2)
    return cc

def p4(p1, p2, p3):
    ''' Given p1 and p2 that make a line, this function returns the points p4 (N,2), that are closest
    to each of the points in p3 (N,2)'''
    x1, y1 = p1[0], p1[1]
    x2, y2 = p2[0], p2[1]
    x3, y3 = p3[:,0], p3[:,1]
    dx, dy = x2-x1, y2-y1
    det = dx*dx + dy*dy
    a = (dy*(y3-y1)+dx*(x3-x1))/det
    a = a[:, None]
    return np.hstack((x1+a*dx, y1+a*dy))

def line_to_gridpts(p1, p2):
    '''Given two points, this function returns all the underlying grid points that
    are nearest to the line connecting them'''
    x1 = int(np.round(p1[0]))
    y1 = int(np.round(p1[1]))
    x2 = int(np.round(p2[0]))
    y2 = int(np.round(p2[1]))
    rr, cc = line(x1, y1, x2, y2)
    return (rr, cc)

def p4_x(x1, y1, x2, y2):
    p1 = np.array([x1, y1])
    p2 = np.array([x2, y2])
    rr, cc = line_to_gridpts(p1, p2) # rr is the row, cc is the column
    p3 = np.hstack((rr[:, None], cc[:, None])) # turn into (N,2) np.array of grid points under the line
    p4s = p4(p1, p2, p3)
    return p4s[:,0]

def p4_y(x, x1, y1, x2, y2):
    p1 = np.array([x1, y1])
    p2 = np.array([x2, y2])
    rr, cc = line_to_gridpts(p1, p2) # rr is the row, cc is the column
    p3 = np.hstack((rr[:, None], cc[:, None])) # turn into (N,2) np.array of grid points under the line
    p4s = p4(p1, p2, p3)
    return p4s[:,1]

def line_draw(x1, y1, x2, y2):
    return np.array([[x1,y1],[x2,y2]])

fig, ax = plt.subplots()
plt.scatter(X,Y, s=0.1, c='k')
controls = iplt.plot(line_draw, c='r', x1 = (1, grid_size-1, 200), y1 = (1, grid_size-1, 200), x2 = (2, grid_size-1, 200), y2 = (2, grid_size-1, 200), parametric=True, xlim = [0, grid_size], ylim=[0, grid_size])
iplt.scatter(linx, liny, c='k', s=10, controls=controls, xlim = [0, grid_size], ylim=[0, grid_size])
iplt.scatter(p4_x, p4_y, c='r', s=10, controls=controls, xlim = [0, grid_size], ylim=[0, grid_size])
plt.show()