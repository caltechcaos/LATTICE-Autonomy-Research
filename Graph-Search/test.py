import numpy as np
from skimage.draw import line

# def line_to_gridpts(p1, p2):
#     '''Given two points, this function returns all the underlying grid points that
#     are nearest to the line connecting them'''
#     # assert p1[0] >= 0 and p1[0] < MAP_W and p1[1] >= 0 and p1[1] < MAP_H, 'P1 out of range!'
#     # assert p2[0] >= 0 and p2[0] < MAP_W and p2[1] >= 0 and p2[1] < MAP_H, 'P2 out of range!'
#     x1 = int(np.round(p1[0]))
#     y1 = int(np.round(p1[1]))
#     x2 = int(np.round(p2[0]))
#     y2 = int(np.round(p2[1]))
#     rr, cc = line(x1, y1, x2, y2)
#     return (rr, cc)


p1 = np.array([[2.5, 2.3, 342], [6, 2.9, 4], [6, 2.9, 4]])
print(p1[[True, False, True]])

# rr, cc = line_to_gridpts(p1, p2)
# print(rr[:,None].shape)
# print(cc.shape)

def get_neighbors(x_c, y_c, r):
    '''Given a point in the grid this function returns the neighboring points
    that are on the circular boundary with the given radius
    Inputs: 
    x_c, y_c are the x and y coordinates (as ints) of the center of the circle
    r is the radius of the circle in grid units (pixels)

    Outputs:
    (N,2) np.array of the N neighboring points on the circle with radius r'''
    assert isinstance(x_c, int) and isinstance(y_c, int), 'Center must be integer coords'
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

neighbors = get_neighbors(0,0,10)
neighbors = neighbors[np.all([neighbors[:,0] >= 0, neighbors[:,1]>= 0], axis = 0)]

import networkx as nx
G = nx.Graph()
G.add_node(np.array([1,2]))