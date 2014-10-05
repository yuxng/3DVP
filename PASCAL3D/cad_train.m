function cad_train

opt = globals();

classes = {'aeroplane', 'bicycle', 'boat', 'bottle', 'bus', 'car', 'chair', ...
    'diningtable', 'motorbike', 'sofa', 'train', 'tvmonitor'};

rescales = {
    [5, 5, 2, 2, 4, 2, 2], ... aeroplane
    0.4 * ones(1,6), ... bicycle
    [2, 1, 2, 5, 1, 5], ... boat
    0.01 * ones(1,8), ... bottle
    [3, 3, 3, 3, 4, 3], ... bus
    ones(1,10), ... car
    0.25 * ones(1,10), ... chair
    0.3 * ones(1,6), ... diningtable
    [0.5, 0.5, 0.4, 0.5, 0.5], ... motorbike
    [0.4, 0.4, 0.4, 0.8, 1, 0.3], ... sofa
    3 * ones(1,4), ... train
    0.15 * ones(1,4) ... tvmonitor
    };

nc = numel(classes);
models = cell(1, nc);
indexes = cell(1, nc);
models_mean = cell(1, nc);
for i = 1:nc
    cls = classes{i};
    filename = sprintf(opt.path_cad, cls);
    disp(filename);
    object = load(filename);
    models{i} = object.(cls);
    
    % load the voxel model
    filename = sprintf('../Geometry/%s.mat', cls);
    object = load(filename);
    voxel = object.(cls);
    index = [];
    for j = 1:numel(voxel)
        models{i}(j).grid_size = voxel(j).grid_size;
        models{i}(j).grid = voxel(j).grid;
        models{i}(j).x3d = voxel(j).x3d;
        models{i}(j).ind = voxel(j).ind;
        if isempty(index)
            index = voxel(j).grid == 1;
        else
            index = index | (voxel(j).grid == 1);
        end
    end
    indexes{i} = index;
    
    % load mean model
    filename = sprintf('../Geometry/%s_mean.mat', cls);
    object = load(filename);
    models_mean{i} = object.(cls);
end

cads.classes = classes;
cads.rescales = rescales;
cads.models = models;
cads.indexes = indexes;
cads.models_mean = models_mean;
save('cads.mat', 'cads');