function cad_train

opt = globals();

classes = {'aeroplane', 'bicycle', 'boat', 'bottle', 'bus', 'car', 'chair', ...
    'diningtable', 'motorbike', 'sofa', 'train', 'tvmonitor'};

rescales = {
    [5, 5, 2, 2, 2, 2, 2], ... aeroplane
    0.5 * ones(1,6), ... bicycle
    [2, 1, 2, 5, 1, 5], ... boat
    0.01 * ones(1,8), ... bottle
    [2, 2, 2, 2, 3, 2], ... bus
    ones(1,10), ... car
    0.25 * ones(1,10), ... chair
    0.3 * ones(1,6), ... diningtable
    [0.5, 0.5, 0.4, 0.5, 0.5], ... motorbike
    [0.5, 0.5, 0.5, 0.8, 1, 0.2], ... sofa
    3 * ones(1,4), ... train
    0.2 * ones(1,4) ... tvmonitor
    };

nc = numel(classes);
models = cell(1, nc);
for i = 1:nc
    cls = classes{i};
    filename = sprintf(opt.path_cad, cls);
    disp(filename);
    object = load(filename);
    models{i} = object.(cls);
end

cads.classes = classes;
cads.rescales = rescales;
cads.models = models;
save('cads.mat', 'cads');