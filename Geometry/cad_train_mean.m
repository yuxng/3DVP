% build the mean voxel model
function cad = cad_train_mean(cls)

grid_size = 50;
switch cls
    case 'aeroplane'
        N = 8;
    case 'bicycle'
        N = 6;
    case 'boat'
        N = 6;
    case 'bottle'
        N = 8;
    case 'bus'
        N = 6;           
    case 'car'
        N = 7;
    case 'chair'
        N = 10;
    case 'diningtable'
        N = 6;
    case 'motorbike'
        N = 5;
    case 'sofa'
        N = 6;
    case 'train'
        N = 4;
    case 'tvmonitor'
        N = 4;           
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

switch cls
    case 'aeroplane'
        aeroplane = cad;
        save('aeroplane_mean.mat', 'aeroplane');
    case 'bicycle'
        bicycle = cad;
        save('bicycle_mean.mat', 'bicycle'); 
    case 'boat'
        boat = cad;
        save('boat_mean.mat', 'boat');
    case 'bottle'
        bottle = cad;
        save('bottle_mean.mat', 'bottle');     
    case 'bus'
        bus = cad;
        save('bus_mean.mat', 'bus');        
    case 'car'
        car = cad;
        save('car_mean.mat', 'car');
    case 'chair'
        chair = cad;
        save('chair_mean.mat', 'chair');       
    case 'diningtable'
        diningtable = cad;
        save('diningtable_mean.mat', 'diningtable');        
    case 'motorbike'
        motorbike = cad;
        save('motorbike_mean.mat', 'motorbike');        
    case 'sofa'
        sofa = cad;
        save('sofa_mean.mat', 'sofa');        
    case 'train'
        train = cad;
        save('train_mean.mat', 'train');        
    case 'tvmonitor'
        tvmonitor = cad;
        save('tvmonitor_mean.mat', 'tvmonitor');        
end