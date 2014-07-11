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
    filename = sprintf('kitti_train/%s_%d_test.mat', cls, cid);
    fprintf('load %s\n', filename);
    object = load(filename);
    boxes1 = object.boxes1;
    boxes_new = cellfun(@(x) process_boxes(x, cid, threshold), boxes1, 'UniformOutput', false);
    if i == 1
        dets = boxes_new;
    else
        dets = cellfun(@(x,y) [x; y], dets, boxes_new, 'UniformOutput', false);
    end
end

filename = sprintf('kitti_train/%s_test.mat', cls);
save(filename, 'dets', '-v7.3');


function boxes_new = process_boxes(boxes, cid, threshold)

if isempty(boxes) == 1
    boxes_new = [];
else
    index= find(boxes(:, 5) > threshold);
    if isempty(index) == 1
        boxes_new = [];
    else
        tmp = boxes(index,:);
        boxes_new = [tmp(:,1:4) cid*ones(size(index)) tmp(:,5)];
    end
end