% build the mean voxel model
function cad = cad_train_mean(cls)

grid_size = 50;
switch cls
    case 'car'
        N = 7;
end

% compute the mean cad model
grid = [];
for i = 1:N
    % load surface model
    filename = sprintf('%s/%02d_surf.off', cls, i);
    disp(filename);
    vertices = load_off_file(filename);
    
    voxel = simple_voxelization(vertices, grid_size);
    if isempty(grid) == 1
        grid = voxel;
    else
        grid = grid + voxel;
    end
end

cad.cls = cls;
cad.grid_size = grid_size;
cad.grid = double(grid > 2);

[x3d, ind] = compute_3d_points(cad.grid);
cad.x3d = x3d;
cad.ind = ind;
fprintf('Build the mean voxel model done\n');