% build the voxel models
function car_sub_train

grid_size = 50;

sub_names = {'hatchback', 'mini', 'sedan', 'SUV', 'wagon'};
sub_nums = [7, 5, 9, 10, 8];

N = sum(sub_nums);
cad(N).vertices = [];
count = 1;
for i = 1:numel(sub_names)
    for j = 1:sub_nums(i)
        % load off file
        filename = sprintf('car_sub/%s_%02d.off', sub_names{i}, j);
        [vertices, faces] = load_off_file(filename);
        cad(count).vertices = vertices;
        cad(count).faces = faces;

        % load surface model
        filename = sprintf('car_sub/%s_%02d_surf.off', sub_names{i}, j);
        disp(filename);
        vertices = load_off_file(filename);

        % voxelization
        grid = simple_voxelization(vertices, grid_size);
        [x3d, ind] = compute_3d_points(grid);

        % save model
        cad(count).grid_size = grid_size;
        cad(count).grid = grid;
        cad(count).x3d = x3d;
        cad(count).ind = ind;
        
        count = count + 1;
    end
end

car = cad;
save('car_sub.mat', 'car');