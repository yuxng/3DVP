function test_contextual_svms

% add liblinear path
addpath('../3rd_party/liblinear-1.94/matlab');

is_train = 1;
is_show = 1;
is_write = 0;
overlap_threshold = 0.5;

% load data
if is_train
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_all.mat');
end
data = object.data;
centers = unique(data.idx_ap);
centers(centers == -1) = [];
nc = numel(centers);

% load contextual svms
models = [];
for i = 1:nc
    cid = centers(i);
    filename = fullfile('contextual_svms', sprintf('car_%d_final.mat', cid));
    object = load(filename);
    if isempty(models)
        models = object.model;
    else
        models(i) = object.model;
    end
end

% load ids
object = load('kitti_ids.mat');
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end

% KITTI path
if is_show
    globals;
    root_dir = KITTIroot;
    if is_train
        data_set = 'training';
    else
        data_set = 'testing';
    end
    cam = 2;
    image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);
end

for i = 1:numel(ids)
    disp(i);
    id = ids(i);
    
    % load detections
    filename = fullfile('CACHED_DATA_TRAINVAL', sprintf('%04d.mat', id));
    object = load(filename);
    features = full(object.features);
    detections = object.detections;
    
    % apply the contextual svms
    num = size(detections, 1);
    labels = zeros(num, 1);
    scores = zeros(num, 1);
    for j = 1:num
        cid = detections(j, 5);
        model = models(centers == cid);
        F = sparse(features(j,:));
        [labels(j), ~, scores(j)] = predict(1, F, model);
    end
    
    if is_write
        % write detections
        if isempty(detections)
            det = [];
        else
            det = [detections(:,1:5) scores];
        end
        
        % non-maximum suppression
        if isempty(det) == 0
            index = nms(det, overlap_threshold);
            det = det(index, :);    
        end        
        
        if is_train == 1
            filename = sprintf('results_kitti_train/%06d.txt', id);
        else
            filename = sprintf('results_kitti_test/%06d.txt', id);
        end
        disp(filename);
        fid = fopen(filename, 'w');

        if isempty(det) == 1
            fprintf('no detection for image %d\n', id);
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
    
    if is_show
        file_img = sprintf('%s/%06d.png', image_dir, id);
        Iimage = imread(file_img);
        dets = detections;
        
        for j = 1:size(dets, 1)
            if labels(j) == -1
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
            bbox = dets(j, 1:4);
            bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            if labels(j) == 1
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