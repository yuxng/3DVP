function exemplar_combine_detectors

cls = 'car';
is_train = 1;
is_pascal = 0;
threshold = -50;
result_dir = 'kitti_train_ap_125';

% load data
if is_pascal
    if is_train == 1
        object = load('../PASCAL3D/data.mat');
    else
        object = load('../PASCAL3D/data_all.mat');
    end    
else
    if is_train == 1
        object = load('../KITTI/data.mat');
    else
        object = load('../KITTI/data_kitti.mat');
    end
end
data = object.data;
idx = data.idx_ap;

centers = double(unique(idx));
centers(centers == -1) = [];
N = numel(centers);
name = sprintf('3d_aps_%d', N);

% load detectors
detectors = cell(1, N);
for i = 1:N   
    filename = sprintf('%s/%s_%s_%d_final.mat', result_dir, cls, name, i);
    disp(filename);
    object = load(filename);
    detector = object.detector;
    detector.opts.pos = [];
    detector.opts.neg = [];
    detector.cid = centers(i);
    detector = acfModify(detector, 'cascThr', threshold);
    detectors{i} = detector;
end

filename = sprintf('%s/%s_%s_combined_detector.mat', result_dir, cls, name);
save(filename, 'detectors', '-v7.3');