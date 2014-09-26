function run_on_test

is_train = 1;
is_show = 0;
is_write = 1;

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

% load weights
object = load('wts_trainval_pascal');
w = object.w;

W_s = w(1:2);
W_a = w(length(W_s)+1:end);

for i = 1:numel(ids)
    disp(i);
    id = ids(i);
    % load detections
    filename = fullfile('CACHED_DATA_TRAINVAL', sprintf('%04d.mat', id));
    object = load(filename, 'Detections', 'Scores', 'Matching', 'Overlaps');
    Tdata.Detections = object.Detections;
    Tdata.Scores = object.Scores;
    Tdata.Matching = object.Matching;
    Tdata.Overlaps = object.Overlaps;

    [I, S] = find_MVC_test(W_s, W_a, centers, Tdata);
    TP = I == 1;
    
    if is_write
        % write detections
        det = [Tdata.Detections S]; 
        
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
        dets = Tdata.Detections;
        scores = S;
        
        for j = 1:size(dets, 1)
            if TP(j) == 0
                continue;
            end
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
            if TP(j) == 1
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