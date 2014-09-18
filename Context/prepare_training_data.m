% prepare the training data to train the contextual model
function prepare_training_data

cls = 'car';
is_train = 1;
cache_dir = 'CACHED_DATA_TRAINVAL';

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

% load detections
if is_train
    filename = sprintf('../ACF/kitti_train_few/%s_test.mat', cls);
else
    filename = sprintf('../ACF/kitti_test_on_train/%s_test.mat', cls);
end
object = load(filename);
dets = object.dets;

% aggregate the detection scores
scores = [];
count = 0;
for i = 1:numel(dets)
    det = dets{i};
    if isempty(det)
        continue;
    end
    num = size(det, 1);
    scores(count+1:count+num) = det(:,6);
    count = count + num;
end
smax = max(scores);
smin = min(scores);
wa = 1 / (smax - smin);
wb = -smin / (smax - smin);
save('scaling_weights.mat', 'wa', 'wb');

% KITTI path
globals;
root_dir = KITTIroot;
if is_train
    data_set = 'training';
else
    data_set = 'testing';
end
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

N = numel(ids);
for i = 1:N
    % read image
    fprintf('%d\n', ids(i));
    file_img = sprintf('%s/%06d.png', image_dir, ids(i));
    I = imread(file_img);
    
    % compute the occlusion patterns
    det = dets{i};
    num = size(det, 1);
    Detections = zeros(num, 5);
    Scores = zeros(num, 1);
    Patterns = cell(num, 1);
    for k = 1:num
        % get predicted bounding box
        bbox_pr = det(k,1:4);
        bbox = zeros(1,4);
        bbox(1) = max(1, round(bbox_pr(1)));
        bbox(2) = max(1, round(bbox_pr(2)));
        bbox(3) = min(size(I,2), round(bbox_pr(3)));
        bbox(4) = min(size(I,1), round(bbox_pr(4)));
        w = bbox(3) - bbox(1) + 1;
        h = bbox(4) - bbox(2) + 1;
        
        Detections(k, 1:4) = bbox;
        Scores(k) = wa * det(k,6) + wb;
        
        % apply the 2D occlusion mask to the bounding box
        % check if truncated pattern
        cid = det(k,5);
        Detections(k, 5) = cid;
        pattern = data.pattern{cid};                
        index = find(pattern == 1);
        if data.truncation(cid) > 0 && isempty(index) == 0
            [y, x] = ind2sub(size(pattern), index);                
            pattern = pattern(min(y):max(y), min(x):max(x));
        end
        Patterns{k} = imresize(pattern, [h w], 'nearest');
    end
    
    filename = fullfile(cache_dir, sprintf('%04d.mat', ids(i)));
    save(filename, 'Detections', 'Scores', 'Patterns');
end