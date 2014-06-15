% check the visibility of voxels
function [visibility_grid, visibility_ind] = check_visibility(cad, azimuth, elevation)

N = cad.grid_size;
grid = cad.grid;
ind = cad.ind;
azimuth = azimuth*pi/180;
elevation = elevation*pi/180;
distance = 2;

% compute the camera center
C = zeros(1,3);
C(1) = distance*cos(elevation)*sin(azimuth);
C(2) = -distance*cos(elevation)*cos(azimuth);
C(3) = distance*sin(elevation);

% visibility flag
num = size(ind, 1);
visibility_ind = zeros(num, 1);
visibility_grid = zeros(N, N, N);
step_size = 1/(10*N);
corner = [0 0 0; 0 0 1; 0 1 0; 0 1 1; 1 0 0; 1 0 1; 1 1 0; 1 1 1];
num_corner = size(corner, 1);

% for each voxel
for k = 1:num
    x = ind(k,1);
    y = ind(k,2);
    z = ind(k,3);         

    % ray tracing for each corner
    dst = C;
    is_occluded = zeros(num_corner, 1);
    for i = 1:num_corner
        src = ([x-1, y-1, z-1] + corner(i,:)) / N - [0.5 0.5 0.5];
        dis = norm(src - dst);
        steps = ceil(dis / step_size);
        for t = 1:steps
            P = src + t*step_size*(dst - src);
            index = floor((P + [0.5 0.5 0.5]) * N) + 1;
            % out of the scope of the grid
            if index(1) <= 0 || index(1) > N || index(2) <= 0 || index(2) > N || ...
                index(3) <= 0 || index(3) > N
                break;
            end
            % meet a non-empty voxel
            if (index(1) ~= x || index(2) ~= y || index(3) ~= z) && grid(index(1), index(2), index(3)) ~= 0
                is_occluded(i) = 1;
                break;
            end
        end
    end
    if sum(is_occluded) == num_corner
        visibility_grid(x, y, z) = 0;
        visibility_ind(k) = 0;
    else
        visibility_grid(x, y, z) = 1;
        visibility_ind(k) = 1;
    end
end