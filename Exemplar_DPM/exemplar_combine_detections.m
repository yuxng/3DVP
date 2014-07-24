function dets = exemplar_combine_detections

cls = 'car';
threshold = -inf;

% load data
object = load('../KITTI/data.mat');
data = object.data;
idx = data.idx;
centers = double(unique(idx));
N = numel(centers);

% load detections
dets = [];
for i = 1:N
    cid = centers(i);
    
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
    
    filename = sprintf('kitti_train/%s_%d_test.mat', cls, cid);
    fprintf('load %s\n', filename);
    object = load(filename);
    boxes1 = object.boxes1;
    boxes_new = cellfun(@(x) process_boxes(x, cid, threshold, lim), boxes1, 'UniformOutput', false);
    if i == 1
        dets = boxes_new;
    else
        dets = cellfun(@(x,y) [x; y], dets, boxes_new, 'UniformOutput', false);
    end
end

filename = sprintf('kitti_train/%s_test.mat', cls);
save(filename, 'dets', '-v7.3');


function boxes_new = process_boxes(boxes, cid, threshold, lim)

if isempty(boxes) == 1
    boxes_new = [];
else
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
        boxes_new = [tmp(:,1:4) cid*ones(size(index)) tmp(:,5)];
    end
end