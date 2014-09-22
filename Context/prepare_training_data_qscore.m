% prepare the training data to train the contextual model
function prepare_training_data_qscore

matlabpool open;
mex compute_contextual_features.cc

cls = 'car';
is_train = 1;
% is_show = 0;
overlap_threshold = 0.7;
cache_dir = 'CACHED_DATA_TRAINVAL';

% KITTI path
% if is_show
%     globals;
%     root_dir = KITTIroot;
%     if is_train
%         data_set = 'training';
%     else
%         data_set = 'testing';
%     end
%     cam = 2;
%     image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);
% end

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
centers = unique(data.idx_ap);
centers(centers == -1) = [];
n = numel(data.imgname);
img_idx = zeros(n, 1);
for i = 1:n
    imgname = data.imgname{i};
    img_idx(i) = str2double(imgname(1:end-4));
end

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

N = numel(ids);
parfor i = 1:N
    fprintf('%d\n', ids(i));  
    
    % get detections
    det = dets{i};
    
    if isempty(det)
        filename = fullfile(cache_dir, sprintf('%04d.mat', ids(i)));
        parsave(filename, [], [], []);
        continue;
    end    
    
    % rescale the detection scores
    det(:,6) = wa .* det(:,6) + wb;
    
    % load the ground truth bounding boxes
    index = img_idx == ids(i);
    gt = data.bbox(:,index)';
    
    % compute labels for detections
    num  = size(det, 1);
    x1 = det(:, 1);
    y1 = det(:, 2);
    x2 = det(:, 3);
    y2 = det(:, 4);  
    ba = (x2-x1+1) .* (y2-y1+1);

    % compute the maximum overlap of each box with each ground truth
    lp = zeros(num, 1);

    % Iterate through ground truth, and update box with new best overlap    
    for j = 1:size(gt,1),
        gx1 = gt(j, 1);
        gy1 = gt(j, 2);
        gx2 = gt(j, 3);
        gy2 = gt(j, 4);
        ga  = (gx2-gx1+1) .* (gy2-gy1+1);

        xx1 = max(x1, gx1);
        yy1 = max(y1, gy1);
        xx2 = min(x2, gx2);
        yy2 = min(y2, gy2);

        w = xx2 - xx1 + 1;
        h = yy2 - yy1 + 1;
        I = find(w > 0 & h > 0);
        int = w(I).*h(I);
        ov  = zeros(num, 1);
        ov(I) = int ./ (ba(I) + ga - int);

        lp = max(lp, ov);
    end
    labels = double(lp > overlap_threshold);
    labels(labels == 0) = -1;
    
    % comput contextual features
    features = compute_contextual_features(det, centers);
    features = sparse(features);
    
    % save features and labels
    filename = fullfile(cache_dir, sprintf('%04d.mat', ids(i)));
    detections = det;
    parsave(filename, labels, features, detections);
    
    % show true detections
%     if is_show
%         file_img = sprintf('%s/%06d.png', image_dir, ids(i));
%         Iimage = imread(file_img);        
%         imshow(Iimage);
%         hold on;
%         for j = 1:num
%             bbox = det(j, 1:4);
%             bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
%             if labels(j) == 1
%                 rectangle('Position', bbox_draw', 'EdgeColor', 'g');
%             else
%                 rectangle('Position', bbox_draw', 'EdgeColor', 'r');
%             end
%             text(bbox(1), bbox(2), num2str(det(j,6)), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
%         end
%         hold off;
%         pause;
%     end    
end

matlabpool close;


function parsave(filename, labels, features, detections)

save(filename, 'labels', 'features', 'detections', '-v7.3');