%create the feature vector for the true labelling of the instance
function extract_groundtruth_features

is_train = 1;
is_show = 0;
overlap_threshold = 0.7;
cache_dir = 'CACHED_DATA_TRAINVAL';
feature_dir = 'FEAT_TRUE_TRAINVAL';

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

% load ids
object = load('kitti_ids_new.mat');
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
centers = unique(data.idx_ap);
img_idx = data.id;
is_flip = data.is_flip;

N = numel(ids);
for id = 1:N
    if is_show
        file_img = sprintf('%s/%06d.png', image_dir, ids(id));
        Iimage = imread(file_img);
    end
    
    filename = fullfile(cache_dir, sprintf('%06d.mat', ids(id)));
    disp(filename);
    object = load(filename, 'Detections', 'Scores', 'Matching', 'Overlaps', 'Idx');
    Detections = object.Detections;
    Detections(:, 6) = 0;
    Scores = object.Scores;
    Matching = object.Matching;
    Overlaps = object.Overlaps;
    Matching(Overlaps < 0.1) = 1;
    Idx = object.Idx;

    % load the ground truth bounding boxes
    % index = find(img_idx == ids(id) & data.idx_ap ~= -1);
    index = find(img_idx == ids(id) & is_flip == 0);
    % for every GT bounding box
    for gi = 1:numel(index)
        ind = index(gi);
        gbox = data.bbox(:, ind);
        
        % cid = data.idx_ap(ind);
        % clsDT = find(Detections(:, 5) == cid & Detections(:, 6) ~= 1);
        clsDT = find(Detections(:, 6) ~= 1);
        n = length(clsDT);
        
        x1 = Detections(clsDT, 1);
        y1 = Detections(clsDT, 2);
        x2 = Detections(clsDT, 3);
        y2 = Detections(clsDT, 4);
        ba = (x2-x1+1) .* (y2-y1+1);

        % pick the one with the highest overlap
        gx1 = gbox(1);
        gy1 = gbox(2);
        gx2 = gbox(3);
        gy2 = gbox(4);
        ga  = (gx2-gx1+1) .* (gy2-gy1+1);

        xx1 = max(x1, gx1);
        yy1 = max(y1, gy1);
        xx2 = min(x2, gx2);
        yy2 = min(y2, gy2);

        w = xx2 - xx1 + 1;
        h = yy2 - yy1 + 1;
        I = find(w > 0 & h > 0);
        int   = w(I).*h(I);
        ov    = zeros(n, 1);
        ov(I) = int ./ (ba(I) + ga - int);
        [v, j] = max(ov);
        
        if v > overlap_threshold
            Detections(clsDT(j), 6) = 1;
        end
        
        %  label as true +ve the detection that overlaps the GT most
        if is_show
            for j = 1:n
                if Detections(clsDT(j), 6) == 0
                    continue;
                end
                bbox = Detections(clsDT(j), 1:4);
                w = bbox(3) - bbox(1) + 1;
                h = bbox(4) - bbox(2) + 1;

                cid = Detections(clsDT(j), 5);
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
        end
    end
    
    % show true detections
    if is_show
        % clustering
        centers_det = unique(Idx);
        n = numel(centers_det);
        
        imshow(Iimage);
        hold on;
        for i = 1:n
            index = find(Idx == centers_det(i));
            ind = find(Detections(index,6) == 1, 1);
            if isempty(ind)
                bbox = Detections(centers_det(i),:);
            else
                bbox = Detections(index(ind),:);
            end
            bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            if bbox(6) == 1
                rectangle('Position', bbox_draw', 'EdgeColor', 'g');
            else
                rectangle('Position', bbox_draw', 'EdgeColor', 'r');
            end
            text(bbox(1), bbox(2), num2str(Scores(centers_det(i))), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
        end
        hold off;
        pause;
    end
 
    % clustering
    idx = Idx;
    centers_det = unique(idx);
    n = numel(centers_det);
    for i = 1:n
        index = find(idx == centers_det(i));
        ind = find(Detections(index,6) == 1, 1);
        if isempty(ind)
            idx(index) = -1;
        end    
    end
    
    % compute the ground truth features    
    [PSI_true, PHI_true] = compute_feature(Detections, Scores, Matching, centers, idx); 
    Feat_true = [PSI_true; PHI_true];
    filename = fullfile(feature_dir, sprintf('%06d.mat', ids(id)));
    save(filename, 'Feat_true');
end