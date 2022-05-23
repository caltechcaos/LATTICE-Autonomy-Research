import random, time, math
from math import sqrt,cos,sin,atan2,pi
from webbrowser import get
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
from skimage.draw import line

# First, set up the map and its related parameters
DATA_STR = 'D:\Dropbox\Projects\LATTICE\LATTICE_Autonomy\Site04_final_adj_5mpp_surf.tif'
MAP = np.array(Image.open(DATA_STR))
MAP_W, MAP_H = MAP.shape    # Width and Height of map   (pixels)
pix2m = 5                   # Conversion factor         (m/pixel)

# Now set the parameters of the LATTICE System
CABLE_SPAN = 100/5             # Distance between stakes   (pixels)
HEIGHT = 0.01                # Cable Heigth above ground (meters)

# Define functions for graph construction
def add_height(points):
    '''Given a (N,2) np.array of points, where the first column is the x position
    and the second columns is the y position, this function returns an (N,3) np.array
    of points, where the third column is the corresponding z-value from MAP'''
    assert all(points[:,0] >= 0) and all(points[:,0] < MAP_W) and all(points[:,1] >= 0) and all(points[:,1] < MAP_H), 'Points out of range!'
    z_vals = MAP[points[:,0].astype(int), points[:,1].astype(int)]
    points = np.hstack((points, z_vals[:,None]))
    return points

def add_line_height(p1, p2, xy_pts):
    '''Given the points p1 and p2 that define a line, this function finds the corresponding
    z-value of each of the xy_pts (N,2), and then returns these xy_points with the third column
    being the corresponding z-height'''
    x1 = p1[0]
    x2 = p2[0]
    y1 = p1[1]
    y2 = p2[1]
    z1 = p1[2]
    z2 = p2[2]
    if x1 == x2:
        m = (z2 - z1) / (y2 - y1)
        z = m*(xy_pts[:,1]-y1) + z1
    else:
        m = (z2 - z1) / (x2 - x1)
        z = m*(xy_pts[:,0]-x1) + z1
    return np.hstack((xy_pts, z[:, None]))

def get_neighbors(x_c, y_c, r):
    '''Given a point in the grid this function returns the neighboring points
    that are on the circular boundary with the given radius
    Inputs: 
    x_c, y_c are the x and y coordinates (as ints) of the center of the circle
    r is the radius of the circle in grid units (pixels)

    Outputs:
    (N,2) np.array of the N neighboring points on the circle with radius r'''
    # assert isinstance(x_c, int) and isinstance(y_c, int), 'Center must be integer coords' + str(x_c) + str(y_c)
    r = np.abs(r) + 0.5
    rads = np.arange(0, np.floor(r * np.sqrt(0.5))+1)[:, None]
    d = np.floor( np.sqrt( r*r - rads*rads  ) )
    d = d.astype(int)
    a1 = np.hstack((x_c - d, y_c + rads))
    a2 = np.hstack((x_c + d, y_c + rads))
    a3 = np.hstack((x_c - d, y_c - rads))
    a4 = np.hstack((x_c + d, y_c - rads))
    a5 = np.hstack((x_c + rads, y_c - d))
    a6 = np.hstack((x_c + rads, y_c + d))
    a7 = np.hstack((x_c - rads, y_c - d))
    a8 = np.hstack((x_c - rads, y_c + d))
    points = np.vstack((a1, a2, a3, a4, a5, a6, a7, a8))
    return np.unique(points, axis=0).astype(int)

def line_to_gridpts(p1, p2):
    '''Given two points, this function returns all the underlying grid points that
    are nearest to the line connecting them'''
    assert p1[0] >= 0 and p1[0] < MAP_W and p1[1] >= 0 and p1[1] < MAP_H, 'P1 out of range!'
    assert p2[0] >= 0 and p2[0] < MAP_W and p2[1] >= 0 and p2[1] < MAP_H, 'P2 out of range!'
    x1 = int(np.round(p1[0]))
    y1 = int(np.round(p1[1]))
    x2 = int(np.round(p2[0]))
    y2 = int(np.round(p2[1]))
    rr, cc = line(x1, y1, x2, y2)
    return (rr, cc)

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

def check_validity(p1, p2):
    '''Given two points passed in as np.arrays of (1,3), including their x, y, z coordinates, 
    this function returns True if a stake placement is valid
    between them'''

    # First we find the points underlying the line between p1 and p2
    rr, cc = line_to_gridpts(p1, p2) # rr is the row, cc is the column
    under_pts = np.hstack((rr[:, None], cc[:, None])) # turn into (N,2) np.array of grid points under the line  
    under_pts = add_height(under_pts) # turn into (N,3) array including z-value heights

    # Find the points (x, y) on the line from p1 to p2 that is closest to each of the under_pts
    p1[2] = p1[2] + HEIGHT
    p2[2] = p2[2] + HEIGHT
    line_pts = p4(p1[0:2], p2[0:2], under_pts[:, 0:2])
    line_pts = add_line_height(p1, p2, line_pts) # turn line_pts into a (N, 3) array including corresponding heights on line

    # Do the check, recall that line_pts are raised by height
    return all(under_pts[:,2] <= line_pts[:,2])

def feasibility_check(origin, neighbors):
    '''Given a list of neighbors (potential stake placement), this function returns
    only the neighbors that are feasible from the given origin.
    Make sure you pass in the origin as a (1,3) np array including x,y,z
    and the points as an (N,3) array.'''
    index = np.full(neighbors.shape[0], False)
    i=0
    for nghbr in neighbors:
        index[i] = check_validity(origin, nghbr)
        i+=1
    return neighbors[index].astype(int)

def get_feasible_neighbors(origin):
    ''' Origin is a (x,y) tuple
    This uses the default CABLE_SPAN set at the top'''
    neighbors = get_neighbors(origin[0], origin[1], CABLE_SPAN)
    # Filter the points out of get_neighbors so that they are in map range
    neighbors = neighbors[np.all([neighbors[:,0] >= 0, neighbors[:,1]>= 0], axis = 0)]
    neighbors = neighbors[np.all([neighbors[:,0] < MAP_W, neighbors[:,1] < MAP_H], axis = 0)]
    neighbors = add_height(neighbors)

    # Add height
    origin = np.array( [ origin[0], origin[1], MAP[origin[0], origin[1]] ] )

    return list(map(tuple, feasibility_check(origin, neighbors)[:,0:2]))

if __name__ ==  '__main__':
    import multiprocessing as mp
    import tqdm

    startTime = time.time() # For timing
    
    # Define the list of points to get the feasible neighbors from
    X, Y = np.mgrid[0:3200:1, 1590:1610:1]
    X = X.astype(int)
    Y = Y.astype(int)
    positions = np.hstack([X.ravel()[:,None], Y.ravel()[:,None]])
    points = list(map(tuple, positions))
    ntasks = len(points)

    with mp.Pool(mp.cpu_count()) as p:
        r = list(tqdm.tqdm(p.imap(get_feasible_neighbors, [pt for pt in points]), total=ntasks))

    executionTime = (time.time() - startTime) # For timing
    print('Execution time in seconds: ' + str(executionTime))

    # Build the graph
    import networkx as nx
    G = nx.Graph()
    G.add_nodes_from(points)
    print('Nodes:')
    print(G.number_of_nodes())

    for i, pt in enumerate(points):
        for nghbr in r[i]:
            G.add_edge(pt, nghbr)

    nx.write_gpickle(G,'0-3200_1598-1602.gpickle')
    
    print('Nodes:')
    print(G.number_of_nodes())
    print(G.number_of_edges())