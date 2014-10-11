function exemplar_combine_detections

globals; 
pascal_init;

cls = 'bicycle';
threshold = -inf;
is_train = 1;
is_calibration = 0;
is_filtering = 1;
is_pascal = 1;
is_pattern = 1;
result_dir = 'data';

% load data
if is_pascal
    if is_train == 1
        object = load('../PASCAL3D/data.mat');
        ids = textread(sprintf(VOCopts.imgsetpath, 'val'), '%s');
    else
        object = load('../PASCAL3D/data_all.mat');
        ids = textread(sprintf(VOCopts.imgsetpath, 'test'), '%s');
    end
    data = object.data;
else
    if is_train == 1
        object = load('../KITTI/data.mat');
    else
        object = load('../KITTI/data_all.mat');
    end
    data = object.data;
    
    object = load('kitti_ids.mat');
    if is_train
        ids = object.ids_val;
    else
        ids = object.ids_test;
    end    
    
    root_dir = KITTIroot;
    data_set = 'training';
    cam = 2; % 2 = left color camera
    image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);    
end
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

if is_pattern
    % apply the segmentation pattern
    M = numel(ids);
    patterns = cell(1, M);
    for i = 1:M
        if isempty(dets{i}) == 1
            continue;
        end
        % read image
        if is_pascal
            file_img = sprintf(VOCopts.imgpath, ids{i});
        else
            file_img = sprintf('%s/%06d.png', image_dir, ids(i));
        end
        disp(file_img);
        I = imread(file_img); 

        % compute the occlusion patterns
        det = dets{i};

        num = size(det, 1);
        pats = cell(1, num);
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

            % apply the 2D occlusion mask to the bounding box
            % check if truncated pattern
            cid = det(k,5);
            pattern = data.pattern{cid};                
            index = find(pattern == 1);
            if is_pascal
                trunc_per = data.trunc_per(cid);
            else
                trunc_per = data.truncation(cid);
            end
            if trunc_per > 0 && isempty(index) == 0
                [y, x] = ind2sub(size(pattern), index);                
                pattern = pattern(min(y):max(y), min(x):max(x));
            end
            pattern = imresize(pattern, [h w], 'nearest');
            
            % modify the bbox
            if is_pascal == 1 && data.occ_per(cid) > 0
                [y, x] = find(pattern == 1);
                dets{i}(k,1) = bbox(1) + min(x) - 1;
                dets{i}(k,2) = bbox(2) + min(y) - 1;
                dets{i}(k,3) = bbox(1) + max(x) - 1;
                dets{i}(k,4) = bbox(2) + max(y) - 1;
            end
            
            pats{k} = pattern;
        end
        patterns{i} = pats;
    end
else
    patterns = [];
end

filename = sprintf('%s/%s_test.mat', result_dir, cls);
save(filename, 'dets', 'patterns', '-v7.3');


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