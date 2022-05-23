import numpy as np

X, Y = np.mgrid[0:10:1, 0:10:1]
positions = np.hstack([X.ravel()[:,None], Y.ravel()[:,None]])
points = list(map(tuple, positions))
print(points)