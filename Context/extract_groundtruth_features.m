%create the feature vector for the true labelling of the instance
function extract_groundtruth_features

is_train = 1;
is_show = 0;
cache_dir = 'CACHED_DATA_TRAINVAL';
feature_dir = 'FEAT_TRUE_TRAINVAL';

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
    file_img = sprintf('%s/%06d.png', image_dir, ids(id));
    Iimage = imread(file_img);
    
    filename = fullfile(cache_dir, sprintf('%04d.mat', ids(id)));
    object = load(filename);
    Detections = object.Detections;
    Detections(:, 6) = 0;
    Scores = object.Scores;
    Patterns = object.Patterns;

    % load the ground truth bounding boxes
    index = find(img_idx == ids(id) & data.idx_ap ~= -1);
    % for every GT bounding box
    for gi = 1:numel(index)
        ind = index(gi);
        cid = data.idx_ap(ind);
        gbox = data.bbox(:, ind);
        
        clsDT = find(Detections(:, 5) == cid & Detections(:, 6) ~= 1);
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
        
        [v,j] = max(ov);
        
        %  label as true +ve the detection that overlaps the GT most
        if v > 0.7
            Detections(clsDT(j), 6) = 1;
            if is_show
                bbox = Detections(clsDT(j), 1:4);
                pattern = Patterns{clsDT(j)};
                im = create_mask_image(pattern);
                Isub = Iimage(bbox(2):bbox(4), bbox(1):bbox(3), :);
                index_pattern = im == 255;
                im(index_pattern) = Isub(index_pattern);
                Iimage(bbox(2):bbox(4), bbox(1):bbox(3), :) = uint8(0.1*Isub + 0.9*im);
            end
        end
    end

    non_bg = find(Detections(:, 6) == 1);
    
    if is_show
        imshow(Iimage);
        hold on;
        for i = 1:numel(non_bg)
            bbox = Detections(non_bg(i), 1:4);
            bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            rectangle('Position', bbox_draw', 'EdgeColor', 'g');
            text(bbox(1), bbox(2), num2str(Scores(non_bg(i))), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
        end
        hold off;
        pause;
    end
 
    if isempty(non_bg)
        Feat_true = [];
    else
        [PSI_true, PHI_true] = compute_feature(Iimage, Detections(non_bg, :), Scores(non_bg), Patterns(non_bg), centers); 
        Feat_true = [PSI_true; PHI_true];
    end    
    filename = fullfile(feature_dir, sprintf('%04d.mat', ids(id)));
    save(filename, 'Feat_true');
end