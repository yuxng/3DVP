function exemplar_combine_detections

cls = 'car';
threshold = -inf;
is_train = 1;
is_calibration = 0;
is_filtering = 1;
is_pascal = 1;
result_dir = 'data';

% load data
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
        object = load('../KITTI/data_all.mat');
    end
end
data = object.data;
idx = data.idx_ap;

centers = double(unique(idx));
centers(centers == -1) = [];
N = numel(centers);

% load detections
dets = [];
for i = 1:N
    cid = centers(i);
    num = numel(find(idx == cid));
    fprintf('cluster %d: %d training examples\n', cid, num);
    
    % compute the range of this cluster
    % compute the range of bounding box center, width and height
    if is_filtering
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
    else
        lim = [];
    end
    
    filename = sprintf('%s/%s_%d_test.mat', result_dir, cls, cid);
    object = load(filename);
    boxes1 = object.boxes1;
    
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

filename = sprintf('%s/%s_test.mat', result_dir, cls);
save(filename, 'dets', '-v7.3');


function boxes_new = process_boxes(boxes, cid, threshold, lim, beta)

if isempty(boxes) == 1
    boxes_new = [];
else
    x = (boxes(:,1) + boxes(:,3)) / 2;
    y = (boxes(:,2) + boxes(:,4)) / 2;
    w = boxes(:,3) - boxes(:,1);
    h = boxes(:,4) - boxes(:,2);    

    if isempty(lim) == 1
        index = 1:size(boxes,1);
        index = index';
    else
        index= find((boxes(:, 5) > threshold & x > lim(1,1) & x < lim(1,2) & ...
            y > lim(2,1) & y < lim(2,2) & w > lim(3,1) & w < lim(3,2) & h > lim(4,1) & h < lim(4,2)) == 1);
    end
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