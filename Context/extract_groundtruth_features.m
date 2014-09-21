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
centers = unique(data.idx_ap);
n = numel(data.imgname);
img_idx = zeros(n, 1);
for i = 1:n
    imgname = data.imgname{i};
    img_idx(i) = str2double(imgname(1:end-4));
end

N = numel(ids);
for id = 1:N
    fprintf('%d\n', ids(id));
    if is_show
        file_img = sprintf('%s/%06d.png', image_dir, ids(id));
        Iimage = imread(file_img);
    end
    
    filename = fullfile(cache_dir, sprintf('%04d.mat', ids(id)));
    object = load(filename);
    Detections = object.Detections;
    Detections(:, 6) = 0;
    Scores = object.Scores;
    Matching = object.Matching;

    % load the ground truth bounding boxes
    % index = find(img_idx == ids(id) & data.idx_ap ~= -1);
    index = find(img_idx == ids(id));
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
        
        % assign all boxes with overlap larger than threshold to 1
        Detections(clsDT(ov > overlap_threshold), 6) = 1;
        
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

    non_bg = find(Detections(:, 6) == 1);
    
    % show true detections
    if is_show
        imshow(Iimage);
        hold on;
        for i = 1:size(Detections,1)
            bbox = Detections(i, 1:4);
            bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            if Detections(i,6) == 1
                rectangle('Position', bbox_draw', 'EdgeColor', 'g');
            else
                rectangle('Position', bbox_draw', 'EdgeColor', 'r');
            end
            text(bbox(1), bbox(2), num2str(Scores(i)), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
        end
        hold off;
        pause;
    end
 
    % compute the ground truth features
    [PSI_true, PHI_true] = compute_feature(Detections(non_bg, :), Scores(non_bg), Matching(non_bg, non_bg), centers); 
    Feat_true = [PSI_true; PHI_true];
    filename = fullfile(feature_dir, sprintf('%04d.mat', ids(id)));
    save(filename, 'Feat_true');
end