% prepare the training data to train the contextual model
function prepare_training_data

matlabpool open;

cls = 'car';
is_train = 1;
overlap_threshold = 0.7;  % use a large threshold for nms
cache_dir = 'CACHED_DATA_TRAINVAL';

% load ids
object = load('kitti_ids.mat');
ids = [object.ids_train object.ids_val];

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
parfor i = 1:N
    % read image
    fprintf('%d\n', ids(i));
    file_img = sprintf('%s/%06d.png', image_dir, ids(i));
    I = imread(file_img);
    width = size(I, 2);
    height = size(I, 1);    
    
    % compute the occlusion patterns
    det = dets{i};
    
    % non-maximum suppression
    if isempty(det) == 0
        index = nms(det, overlap_threshold);
        det = det(index, :);    
    end
    
    num = size(det, 1);
    Detections = zeros(num, 5);
    Scores = zeros(num, 1);
    Patterns = uint8(zeros(height, width, num));
    for k = 1:num
        % get predicted bounding box
        bbox_pr = det(k,1:4);
        bbox = zeros(1,4);
        bbox(1) = max(1, floor(bbox_pr(1)));
        bbox(2) = max(1, floor(bbox_pr(2)));
        bbox(3) = min(size(I,2), floor(bbox_pr(3)));
        bbox(4) = min(size(I,1), floor(bbox_pr(4)));
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
        pattern = imresize(pattern, [h w], 'nearest');
        
        % build the pattern in the image
        P = uint8(zeros(height, width));
        x = bbox(1);
        y = bbox(2);
        index_y = y:min(y+h-1, height);
        index_x = x:min(x+w-1, width);
        P(index_y, index_x) = pattern(1:numel(index_y), 1:numel(index_x));
        Patterns(:,:,k) = P;
    end
    
    % precompute the pattern matching scores
%     Matching = zeros(num, num);
%     for j = 1:num
%         dj = Detections(j,4);   
%         pj = Patterns(:,:,j);
%         for k = j+1:num
%             dk = Detections(k,4);
%             pk = Patterns(:,:,k);
% 
%             index = pj > 0 & pk > 0;
%             overlap = sum(sum(index));
% 
%             if dj < dk  % object j is occluded
%                 rj = overlap / sum(sum(pj > 0));
%                 if rj < 0.1
%                     s = 1;
%                 else
%                     s = sum(pj(index) == 2) / overlap;
%                 end
%             else  % object k is occluded
%                 rk = overlap / sum(sum(pk > 0));            
%                 if rk < 0.1
%                     s = 1;
%                 else
%                     s = sum(pk(index) == 2) / overlap;
%                 end
%             end
%             
%             Matching(j, k) = s;
%             Matching(k, j) = s;
%         end
%     end
    
    Matching = compute_matching_scores(Detections, Patterns);
    
    filename = fullfile(cache_dir, sprintf('%04d.mat', ids(i)));
    parsave(filename, Detections, Scores, Matching);
end

matlabpool close;

function parsave(filename, Detections ,Scores, Matching)

save(filename, 'Detections', 'Scores', 'Matching', '-v7.3');