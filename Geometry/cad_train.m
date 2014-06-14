% build the voxel models
function cad_train(cls)

grid_size = 50;
switch cls
    case 'car'
        N = 10;
end

cad(N).vertices = [];
for i = 1:N
    % load off file
    filename = sprintf('%s/%02d.off', cls, i);
    [vertices, faces] = load_off_file(filename);
    cad(i).vertices = vertices;
    cad(i).faces = faces;
    
    % load surface model
    filename = sprintf('%s/%02d_surf.off', cls, i);
    disp(filename);
    vertices = load_off_file(filename);
    
    % voxelization
    grid = simple_voxelization(vertices, grid_size);
    [x3d, ind] = compute_3d_points(grid);
    
    % save model
    cad(i).grid_size = grid_size;
    cad(i).grid = grid;
    cad(i).x3d = x3d;
    cad(i).ind = ind;
end

switch cls
    case 'car'
        car = cad;
        save('car.mat', 'car');
end