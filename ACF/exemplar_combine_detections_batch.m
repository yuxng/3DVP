function exemplar_combine_detections_batch

cls = 'car';
threshold = -inf;
is_train = 1;
is_filtering = 1;
is_pascal = 0;
is_2d = 1;
is_kmeans = 0;

if is_kmeans
    result_dir = 'kitti_train_kmeans_new';
else
    result_dir = 'kitti_train_aps_new';
end

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

if is_kmeans
    K = data.K;
    num = numel(K);
else
    if is_2d
        num = numel(data.idx_2d_aps);
    else
        num = numel(data.idx_3d_aps);
    end
end

for k = 1:num
    if is_kmeans
        if is_2d
            idx = data.idx_2d_kmeans{k};
            name = sprintf('2d_kmeans_%d', K(k));
            if K(k) == 5
                threshold = 1;
            end
        else
            idx = data.idx_3d_kmeans{k};
            name = sprintf('3d_kmeans_%d', K(k));
        end
    else
        if is_2d
            idx = data.idx_2d_aps{k};
            name = sprintf('2d_aps_%d', k);
        else
            idx = data.idx_3d_aps{k};
            name = sprintf('3d_aps_%d', k);
        end
    end

    centers = double(unique(idx));
    centers(centers == -1) = [];
    N = numel(centers);

    % load detections
    dets = [];
    for i = 1:N
        cid = centers(i);
        num = numel(find(idx == cid));
        fprintf('%s cluster %d: %d training examples\n', name, i, num);

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

        filename = sprintf('%s/%s_%s_%d_test.mat', result_dir, cls, name, i);
        if exist(filename, 'file') == 0
            continue;
        end
        object = load(filename);
        boxes1 = object.boxes;
        
        if is_2d
            if size(boxes1{1},1) > 1000
                threshold = -1;
            else
                threshold = -50;
            end
        end

        % load the calibration weights
        beta = [];
        boxes_new = cellfun(@(x) process_boxes(x, cid, threshold, lim, beta), boxes1, 'UniformOutput', false);
        if isempty(dets) == 1
            dets = boxes_new;
        else
            dets = cellfun(@(x,y) [x; y], dets, boxes_new, 'UniformOutput', false);
        end
    end

    filename = sprintf('%s/%s_%s_combined_test.mat', result_dir, cls, name);
    save(filename, 'dets', '-v7.3');
    
    write_kitti_results(dets, data, is_train);
    exemplar_compute_aps;
    pause;
end


function boxes_new = process_boxes(boxes, cid, threshold, lim, beta)

if isempty(boxes) == 1
    boxes_new = [];
else
    boxes = [boxes(:,1) boxes(:,2) boxes(:,1)+boxes(:,3) boxes(:,2)+boxes(:,4) boxes(:,5)];
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


function write_kitti_results(dets, data, is_train)

threshold_overlap = 0.6;

% read ids of validation images
object = load('kitti_ids_new.mat');
if is_train == 1
    ids = object.ids_val;
else
    ids = object.ids_test;
end
N = numel(ids);

for i = 1:N
    img_idx = ids(i);    
    det = dets{i};
    
    % result file
    if is_train == 1
        filename = sprintf('results_kitti_train/%06d.txt', img_idx);
    else
        filename = sprintf('results_kitti_test/%06d.txt', img_idx);
    end
    disp(filename);
    fid = fopen(filename, 'w');
    
    if isempty(det) == 1
        fprintf('no detection for image %d\n', img_idx);
        fclose(fid);
        continue;
    end
    
    % non-maximum suppression
    if isempty(det) == 0
        I = nms_new(det, threshold_overlap);
        det = det(I, :);    
    end    
    
    % write detections
    num = size(det, 1);
    for k = 1:num
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