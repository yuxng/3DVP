function exemplar_greedy_occlusion_reasoning_nms

% matlabpool open;

is_train = 1;
is_show = 0;
is_write = 1;
K = 10;   % number of detections to keep in each cluster
nms_threshold = 0.6;

cache_dir = 'CACHED_DATA_TRAINVAL';

addpath(genpath('../KITTI'));

% load data
if is_train
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_all.mat');
end
data = object.data;

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

% read ids of validation images
object = load('kitti_ids_new.mat');
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end
N = numel(ids);

for id = 1:N
    disp(id);
    % read image
    if is_show
        file_img = sprintf('%s/%06d.png', image_dir, ids(id));
        Iimage = imread(file_img);      
        fprintf('%06d\n', ids(id));
    end

    % load detection results
    filename = fullfile(cache_dir, sprintf('%06d.mat', ids(id)));
    record = load(filename, 'Detections', 'Scores', 'Overlaps', 'Matching');
    detections = record.Detections;
    scores = record.Scores;
    overlaps = record.Overlaps;
    matching = record.Matching;
    num = numel(scores);
    
    if num == 0
        if is_write
            if is_train == 1
                filename = sprintf('results_kitti_train/%06d.txt', ids(id));
            else
                filename = sprintf('results_kitti_test/%06d.txt', ids(id));
            end
            fid = fopen(filename, 'w');
            fclose(fid);
        end        
        continue;
    end
    
    % nms clustering on bounding boxes
    det = [detections scores];
    idx = nms_clustering(det, nms_threshold, K); 
    
    % for each cluster, keep the top K detections only
    % max pooling on detection scores in each cluster, but use the most
    % compatible pattern
    centers = unique(idx);
    centers(centers == -1) = [];
    n = numel(centers);
    clusters = cell(n, 1);
    scores_cluster = zeros(n, 1);
    index_cluster = zeros(n, 1);    
    for i = 1:n
        index = find(idx == centers(i));
        clusters{i} = index;
        % sort the detections in each cluster
        [~, ind] = sort(scores(index), 'descend');
        index = index(ind);
        scores_cluster(i) = scores(index(1));
        index_cluster(i) = index(1);
    end

    % sort clusters by detection scores
    fprintf('%d clusters\n', n);
    [~, index] = sort(scores_cluster, 'descend');

    % add clusters into the scene greedily
    for i = 1:n
        ci = index(i);
        % find the most compatible pattern
        cindex = clusters{ci};
        s = [];
        for j = 1:i-1
            cj = index(j);
            % for each objects in the scene
            M = matching(cindex, index_cluster(cj));
            O = overlaps(cindex, index_cluster(cj));
            if min(O) > 0.2
                s = [s M];
            end
        end
        % choose the most compatible pattern
        if isempty(s) == 0
            s = sum(s, 2);
            [~, ind] = max(s);
            index_cluster(ci) = cindex(ind);
        end
    end
    
    % compute the flags for objects
    flags = zeros(1, num);
    det = [];
    count = 0;
    for i = 1:n
        flags(index_cluster(i)) = 1;
        count = count + 1;
        det(count,:) = [detections(index_cluster(i),:) scores_cluster(i)];
    end
    
    if is_write
        % write detections 
        if is_train == 1
            filename = sprintf('results_kitti_train/%06d.txt', ids(id));
        else
            filename = sprintf('results_kitti_test/%06d.txt', ids(id));
        end
        disp(filename);
        fprintf('%d objects detected\n', size(det, 1));
        fid = fopen(filename, 'w');

        if isempty(det) == 1
            fprintf('no detection for image %d\n', ids(id));
            fclose(fid);
            continue;
        end

        num = size(det, 1);
        for k = 1:num
            if isinf(det(k,6))
                continue;
            end
            cid = det(k, 5);
            truncation = data.truncation(cid);

            occ_per = data.occ_per(cid);
            if occ_per > 0.5
                occlusion = 2;
            elseif occ_per > 0
                occlusion = 1;
            else
                occlusion = 0;
            end

            azimuth = data.azimuth(cid);
            alpha = azimuth + 90;
            if alpha >= 360
                alpha = alpha - 360;
            end
            alpha = alpha*pi/180;
            if alpha > pi
                alpha = alpha - 2*pi;
            end

            h = data.h(cid);
            w = data.w(cid);
            l = data.l(cid);

            fprintf(fid, '%s %f %d %f %f %f %f %f %f %f %f %f %f %f %f %f\n', ...
                'Car', truncation, occlusion, alpha, det(k,1), det(k,2), det(k,3), det(k,4), ...
                h, w, l, -1, -1, -1, -1, det(k,6));
        end
        fclose(fid);
    end    

%     if is_show
%         % show box clustering result
%         figure(1);
%         cmap = colormap;
%         imshow(Iimage);
%         hold on;
%         for i = 1:n
%             index = clusters{i};
%             for j = 1:numel(index)
%                 bbox = detections(index(j), 1:4);
%                 bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
%                 cindex = round(i * size(cmap,1) / n);
%                 rectangle('Position', bbox_draw', 'EdgeColor', cmap(cindex,:));
%                 text(bbox(1), bbox(2), num2str(i), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
%             end
%         end
%         hold off;        
%         
%         % show occlusion reasoning result
%         dets = detections;
%         for j = 1:size(dets, 1)
%             if flags(j) == 0
%                 continue;
%             end
%             
%             bbox_pr = dets(j,1:4);
%             bbox = zeros(1,4);
%             bbox(1) = max(1, floor(bbox_pr(1)));
%             bbox(2) = max(1, floor(bbox_pr(2)));
%             bbox(3) = min(size(Iimage,2), floor(bbox_pr(3)));
%             bbox(4) = min(size(Iimage,1), floor(bbox_pr(4)));
%             w = bbox(3) - bbox(1) + 1;
%             h = bbox(4) - bbox(2) + 1;
% 
%             c = dets(j, 5);
%             pattern = data.pattern{c};                
%             index_pattern = find(pattern == 1);
%             if data.truncation(c) > 0 && isempty(index_pattern) == 0
%                 [y, x] = ind2sub(size(pattern), index_pattern);                
%                 pattern = pattern(min(y):max(y), min(x):max(x));
%             end
%             pattern = imresize(pattern, [h w], 'nearest');                
% 
%             im = create_mask_image(pattern);
%             Isub = Iimage(bbox(2):bbox(4), bbox(1):bbox(3), :);
%             index_pattern = im == 255;
%             im(index_pattern) = Isub(index_pattern);
%             Iimage(bbox(2):bbox(4), bbox(1):bbox(3), :) = uint8(0.1*Isub + 0.9*im);
%         end
%         
%         figure(2);
%         imshow(Iimage);
%         hold on;
%         for j = 1:size(dets, 1)
%             if flags(j) == 0
%                 continue;
%             end            
%             bbox = dets(j, 1:4);
%             bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
%             rectangle('Position', bbox_draw', 'EdgeColor', 'g');
%             s = sprintf('%d:%.2f', flags(j), scores_cluster(index_cluster == j));
%             text(bbox(1), bbox(2), s, 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
%         end
%         hold off;
%         pause;   
%     end
end

% matlabpool close;