% build the voxel models
function car_sub_train_mean

grid_size = 50;

sub_names = {'hatchback', 'mini', 'sedan', 'SUV', 'wagon'};
sub_nums = [7, 5, 9, 10, 8];

% compute the mean cad model
grid = [];
for i = 1:numel(sub_names)
    for j = 1:sub_nums(i)

        % load surface model
        filename = sprintf('car_sub/%s_%02d_surf.off', sub_names{i}, j);
        disp(filename);
        vertices = load_off_file(filename);

        % voxelization
        voxel = simple_voxelization(vertices, grid_size);
        if isempty(grid) == 1
            grid = voxel;
        else
            grid = grid + voxel;
        end
    end
end

cad.cls = 'car';
cad.grid_size = grid_size;
cad.grid = double(grid > 2);

[x3d, ind] = compute_3d_points(cad.grid);
cad.x3d = x3d;
cad.ind = ind;
fprintf('Build the mean voxel model done\n');

car = cad;
save('car_sub_mean.mat', 'car');