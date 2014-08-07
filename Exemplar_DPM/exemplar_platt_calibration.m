function exemplar_platt_calibration

cls = 'car';
is_train = 0;

% load data
if is_train == 1
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_trainval.mat');
end
data = object.data;

% convert image names to ids
imgname = data.imgname;
num = numel(imgname);
ids_gt = zeros(1, num);
for i = 1:num
    filename = imgname{i};
    ids_gt(i) = str2double(filename(1:end-4));
end

% read ids of validation images
object = load('kitti_ids.mat');
if is_train == 1
    ids = object.ids_train;
else
    ids = [object.ids_train object.ids_val];
end
M = numel(ids);

idx = data.idx;
centers = double(unique(idx));
N = numel(centers);

% for each cluster
for i = 1:N
    cid = centers(i);
    
    % load detection results
    if is_train == 1
        filename = sprintf('kitti_train/%s_%d_test.mat', cls, cid);
    else
        filename = sprintf('kitti_test/%s_%d_train.mat', cls, cid);
    end
    object = load(filename);
    boxes = object.boxes1;
    
    scores = [];
    os = [];
    count = 0;
    for j = 1:M
        id = ids(j);
        bbox_pr = boxes{id+1};
        % find ground truth boxes
        flag = ids_gt == id & idx' == cid;
        bbox_gt = data.bbox(:,flag == 1)';
        % compute overlap
        for k = 1:min(size(bbox_pr,1), 100)
            count = count + 1;
            scores(count) = bbox_pr(k,5);
            overlap = boxoverlap(bbox_gt, bbox_pr(k,1:4));
            if isempty(overlap) == 1
                os(count) = 0;
            else
                os(count) = max(overlap);
            end
        end
    end
    
    % learn the sigmod function
    beta = exemplar_learn_sigmoid(scores, os);
    if is_train == 1
        filename = sprintf('kitti_train/%s_%d_calib.mat', cls, cid);
    else
        filename = sprintf('kitti_test/%s_%d_calib.mat', cls, cid);
    end
    disp(filename);
    save(filename, 'beta');    
end