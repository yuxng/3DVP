% build the voxel models
function cad_train(cls)

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
    case 'aeroplane'
        aeroplane = cad;
        save('aeroplane.mat', 'aeroplane');
    case 'bicycle'
        bicycle = cad;
        save('bicycle.mat', 'bicycle'); 
    case 'boat'
        boat = cad;
        save('boat.mat', 'boat');
    case 'bottle'
        bottle = cad;
        save('bottle.mat', 'bottle');     
    case 'bus'
        bus = cad;
        save('bus.mat', 'bus');        
    case 'car'
        car = cad;
        save('car.mat', 'car');
    case 'chair'
        chair = cad;
        save('chair.mat', 'chair');       
    case 'diningtable'
        diningtable = cad;
        save('diningtable.mat', 'diningtable');        
    case 'motorbike'
        motorbike = cad;
        save('motorbike.mat', 'motorbike');        
    case 'sofa'
        sofa = cad;
        save('sofa.mat', 'sofa');        
    case 'train'
        train = cad;
        save('train.mat', 'train');        
    case 'tvmonitor'
        tvmonitor = cad;
        save('tvmonitor.mat', 'tvmonitor');        
end