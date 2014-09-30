function cad_train

opt = globals();

classes = {'aeroplane', 'bicycle', 'boat', 'bottle', 'bus', 'car', 'chair', ...
    'diningtable', 'motorbike', 'sofa', 'train', 'tvmonitor'};

nc = numel(classes);
cads = cell(nc, 1);
for i = 1:nc
    cls = classes{i};
    filename = sprintf(opt.path_cad, cls);
    object = load(filename);
    cads{i} = object.(cls);
end
save('cads.mat', 'cads');