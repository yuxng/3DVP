function exemplar_greedy_occlusion_reasoning

% matlabpool open;

is_train = 1;
is_show = 1;
cache_dir = 'CACHED_DATA_TRAINVAL';

addpath(genpath('../KITTI'));

if is_show
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
    
    % load data
    if is_train
        object = load('../KITTI/data.mat');
    else
        object = load('../KITTI/data_all.mat');
    end
    data = object.data;
end

% read ids of validation images
object = load('kitti_ids.mat');
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end
M = numel(ids);

for id = 1:M
    disp(id);
    % read image
    fprintf('%d\n', ids(id));

    % load detection results
    filename = fullfile(cache_dir, sprintf('%04d.mat', ids(id)));
    record = load(filename);
    detections = record.Detections;
    scores = record.Scores;
    overlaps = record.Overlaps;
    matching = record.Matching;
    num = numel(scores);
    
    if num == 0
        continue;
    end
    
    if num == 1
        idx = 1;
    else
        % meanshift clustering on bounding boxes
        x = [(detections(:,1)+detections(:,3))/2 (detections(:,2)+detections(:,4))/2]';
        bandwidth = 20;
        [~, idx, ~] = MeanShiftCluster(x, bandwidth);
    end
    
    % sort clusters
    cid = unique(idx);
    n = numel(cid);
    fprintf('%d clusters\n', n);
    scores_cluster = zeros(1, n);
    for i = 1:n
        scores_cluster(i) = max(scores(idx == cid(i)));
    end
    [~, index] = sort(scores_cluster, 'descend');
    cid = cid(index);

    % add clusters into the scene greedily
    flags = zeros(1, n);
    for i = 1:n
        flags(i) = 1;
        for j = 1:i-1
            % for each objects in the scene
            if flags(j) == 1
                M = matching(idx == cid(i), idx == cid(j));
                smax = max(max(M));
                if smax < 0.5  % incompatitable
                    flags(i) = 0;
                    break;
                end
            end
        end
    end
    
    % compute the flags for objects
    flags_cluster = flags;
    flags = zeros(1, num);
    for i = 1:n
        if flags_cluster(i) == 1
            index = find(idx == cid(i));
            [~, ind] = max(scores(index));
            flags(index(ind)) = 1;
        end
    end

    if is_show
        file_img = sprintf('%s/%06d.png', image_dir, ids(id));
        Iimage = imread(file_img);
        dets = detections;
        
        for j = 1:size(dets, 1)
            if flags(j) == 0
                continue;
            end
            
            bbox_pr = dets(j,1:4);
            bbox = zeros(1,4);
            bbox(1) = max(1, floor(bbox_pr(1)));
            bbox(2) = max(1, floor(bbox_pr(2)));
            bbox(3) = min(size(Iimage,2), floor(bbox_pr(3)));
            bbox(4) = min(size(Iimage,1), floor(bbox_pr(4)));
            w = bbox(3) - bbox(1) + 1;
            h = bbox(4) - bbox(2) + 1;

            cid = dets(j, 5);
            pattern = data.pattern{cid};                
            index_pattern = find(pattern == 1);
            if data.truncation(cid) > 0 && isempty(index_pattern) == 0
                [y, x] = ind2sub(size(pattern), index_pattern);                
                pattern = pattern(min(y):max(y), min(x):max(x));
            end
            pattern = imresize(pattern, [h w], 'nearest');                

            im = create_mask_image(pattern);
            Isub = Iimage(bbox(2):bbox(4), bbox(1):bbox(3), :);
            index_pattern = im == 255;
            im(index_pattern) = Isub(index_pattern);
            Iimage(bbox(2):bbox(4), bbox(1):bbox(3), :) = uint8(0.1*Isub + 0.9*im);
        end
        
        imshow(Iimage);
        hold on;
        for j = 1:size(dets, 1)
            if flags(j) == 0
                continue;
            end            
            bbox = dets(j, 1:4);
            bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            if flags(j) == 1
                rectangle('Position', bbox_draw', 'EdgeColor', 'g');
            else
                rectangle('Position', bbox_draw', 'EdgeColor', 'r');
            end
            text(bbox(1), bbox(2), num2str(scores(j)), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
        end
        hold off;
        pause;   
    end
end

% matlabpool close;