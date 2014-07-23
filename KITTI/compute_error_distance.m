% compute the projection error between 3D bbox and 2D bbox
function error = compute_error_distance(x, vertices, C, X, P, Pv2c, bw, bh)

% 3D bounding box dimensions
object.l = x(1);
object.h = x(2);
object.w = x(3);
object.ry = x(4);

% compute the translate of the 3D bounding box
t = C + x(5) .* X;
t = Pv2c*[t; 1];
t(4) = [];
object.t = t;

% compute 3D points
x3d = compute_3d_points(vertices, object);

% project the 3D bounding box into the image plane
x2d = projectToImage(x3d, P);

% compute bounding box width and height
width = max(x2d(1,:)) - min(x2d(1,:));
height = max(x2d(2,:)) - min(x2d(2,:));

% compute error
error = (width - bw)^2 + (height - bh)^2;