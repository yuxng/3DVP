function run_on_test

is_train = 1;
is_show = 1;

% load data
if is_train
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_all.mat');
end
data = object.data;
centers = unique(data.idx_ap);

% load ids
object = load('kitti_ids.mat');
if is_train
    ids = object.ids_train;
else
    ids = [object.ids_train object.ids_val];
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

% load weights
object = load('wts_trainval_pascal');
w = object.w;

W_s = w(1:2);
W_a = w(length(W_s)+1:end);

for i = 1:numel(ids)
    id = ids(i);
    % load detections
    filename = fullfile('CACHED_DATA_TRAINVAL', sprintf('%04d.mat', id));
    object = load(filename);
    Tdata.Detections = object.Detections;
    Tdata.Scores = object.Scores;
    Tdata.Matching = object.Matching;

    [I, S] = find_MVC_test(W_s, W_a, centers, Tdata);
    TP = I == 1;
    
    if is_show
        file_img = sprintf('%s/%06d.png', image_dir, id);
        Iimage = imread(file_img);
        dets = Tdata.Detections(TP,:);
        scores = Tdata.Scores(TP);
        
        for j = 1:size(dets, 1)
            bbox = dets(j, 1:4);
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
            rectangle('Position', bbox_draw', 'EdgeColor', 'g');
            text(bbox(1), bbox(2), num2str(scores(j)), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
        end
        hold off;
        pause;        
    end
end