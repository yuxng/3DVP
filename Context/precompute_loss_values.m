function precompute_loss_values

is_train = 1;
overlap_threshold = 0.7;
cache_dir = 'CACHED_DATA_TRAINVAL';
loss_dir = 'LOSS_TRAINVAL';

% load ids
object = load('kitti_ids.mat');
if is_train
    ids = object.ids_train;
else
    ids = [object.ids_train object.ids_val];
end

% load data
if is_train
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_all.mat');
end
data = object.data;
n = numel(data.imgname);
img_idx = zeros(n, 1);
for i = 1:n
    imgname = data.imgname{i};
    img_idx(i) = str2double(imgname(1:end-4));
end

N = numel(ids);
for id = 1:N
    fprintf('%d\n', ids(id));    
    % load the ground truth bounding boxes
    % index = find(img_idx == ids(id) & data.idx_ap ~= -1);
    index = find(img_idx == ids(id));
    GT = [data.bbox(:,index)' data.idx_ap(index)];
    
    % load detections
    filename = fullfile(cache_dir, sprintf('%04d.mat', ids(id)));
    object = load(filename, 'Detections');
    Detections = object.Detections;
        
    loss = compute_loss(Detections, GT, overlap_threshold);
        
    filename = fullfile(loss_dir, sprintf('%04d.mat', ids(id)));
    save(filename, 'loss');
end