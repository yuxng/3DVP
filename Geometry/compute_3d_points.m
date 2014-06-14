% compute the 3D point location for voxel
function [x3d, ind] = compute_3d_points(grid)

grid_size = size(grid, 1);
index = find(grid(:) == 1);

N = numel(index);
x3d = zeros(N, 3);
ind = zeros(N, 3);
for i = 1:N
    [x, y, z] = ind2sub([grid_size grid_size grid_size], index(i));
    x3d(i,:) = [x-0.5 y-0.5 z-0.5] / grid_size - 0.5;
    ind(i,:) = [x y z];
end