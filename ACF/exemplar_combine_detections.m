function exemplar_combine_detections

cls = 'car';
threshold = -inf;
is_train = 0;
is_calibration = 0;

% load data
if is_train == 1
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_all.mat');
end
data = object.data;
idx = data.idx_ap;
centers = double(unique(idx));
N = numel(centers);

% load detections
dets = [];
for i = 1:N
    cid = centers(i);
    num = numel(find(idx == cid));
    fprintf('cluster %d: %d training examples\n', cid, num);
    
    % compute the range of this cluster
    % compute the range of bounding box center, width and height
    bbox = data.bbox(:, idx == cid);
    x = (bbox(1,:) + bbox(3,:)) / 2;
    y = (bbox(2,:) + bbox(4,:)) / 2;
    w = bbox(3,:) - bbox(1,:);
    h = bbox(4,:) - bbox(2,:);
    r = [x; y; w; h];
    rmin = min(r, [], 2);
    rmax = max(r, [], 2);
    rlen = rmax - rmin;
    lim = [rmin - 0.1*rlen rmax + 0.1*rlen];
    
    if is_train == 1
        filename = sprintf('kitti_train/%s_%d_test.mat', cls, cid);
    else
        filename = sprintf('kitti_test/%s_%d_test.mat', cls, cid);
    end
    object = load(filename);
    boxes1 = object.boxes;
    
    % load the calibration weights
    if is_calibration == 1
        if is_train == 1
            filename = sprintf('kitti_train/%s_%d_calib.mat', cls, cid);
        else
            filename = sprintf('kitti_test/%s_%d_calib.mat', cls, cid);
        end
        if exist(filename, 'file') == 0
            beta = [];
        else
            object = load(filename);
            beta = object.beta;
        end
    else
        beta = [];
    end
    
    boxes_new = cellfun(@(x) process_boxes(x, cid, threshold, lim, beta), boxes1, 'UniformOutput', false);
    if isempty(dets) == 1
        dets = boxes_new;
    else
        dets = cellfun(@(x,y) [x; y], dets, boxes_new, 'UniformOutput', false);
    end
end

if is_train == 1
    filename = sprintf('kitti_train/%s_test.mat', cls);
else
    filename = sprintf('kitti_test/%s_test.mat', cls);
end
save(filename, 'dets', '-v7.3');


function boxes_new = process_boxes(boxes, cid, threshold, lim, beta)

if isempty(boxes) == 1
    boxes_new = [];
else
    boxes = [boxes(:,1) boxes(:,2) boxes(:,1)+boxes(:,3) boxes(:,2)+boxes(:,4) boxes(:,5)];
    x = (boxes(:,1) + boxes(:,3)) / 2;
    y = (boxes(:,2) + boxes(:,4)) / 2;
    w = boxes(:,3) - boxes(:,1);
    h = boxes(:,4) - boxes(:,2);    
    
    index= find((boxes(:, 5) > threshold & x > lim(1,1) & x < lim(1,2) & ...
        y > lim(2,1) & y < lim(2,2) & w > lim(3,1) & w < lim(3,2) & h > lim(4,1) & h < lim(4,2)) == 1);
    if isempty(index) == 1
        boxes_new = [];
    else
        tmp = boxes(index,:);
        scores = tmp(:,5);
        if isempty(beta) == 1
            s = scores;
        else
            s = 1./(1+exp(-beta(1)*(scores-beta(2))));
        end
        boxes_new = [tmp(:,1:4) cid*ones(size(index)) s];
    end
end