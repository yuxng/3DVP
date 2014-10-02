function cad_train

opt = globals();

classes = {'aeroplane', 'bicycle', 'boat', 'bottle', 'bus', 'car', 'chair', ...
    'diningtable', 'motorbike', 'sofa', 'train', 'tvmonitor'};

rescales = [3, 0.2, 2, 0.01, 2, 1, 0.1, 0.5, 0.2, 0.5, 3, 0.1];

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