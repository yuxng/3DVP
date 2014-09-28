function exemplar_display_result_kitti

cls = 'car';
threshold = -20;
is_train = 1;
threshold_overlap = 0.6;
result_dir = 'kitti_train_kmeans_2d_100';

% read detection results
if is_train
    filename = sprintf('%s/%s_test.mat', result_dir, cls);
else
    filename = sprintf('kitti_test_few/%s_test.mat', cls);
end
object = load(filename);
dets = object.dets;
fprintf('load detection done\n');

% read ids of validation images
object = load('kitti_ids.mat');
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end
N = numel(ids);

% KITTI path
exemplar_globals;
root_dir = KITTIroot;
if is_train
    data_set = 'training';
else
    data_set = 'testing';
end
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);
label_dir = fullfile(root_dir, [data_set '/label_' num2str(cam)]);

figure;
for i = 1:N
    img_idx = ids(i);
    disp(img_idx);
    
    % read ground truth bounding box
    if is_train
        objects = readLabels(label_dir, img_idx);
        clsinds = strmatch(cls, lower({objects(:).type}), 'exact');
        n = numel(clsinds);
        bbox = zeros(n, 4); 
        for j = 1:n
            bbox(j,:) = [objects(clsinds(j)).x1 objects(clsinds(j)).y1 ...
                objects(clsinds(j)).x2 objects(clsinds(j)).y2];     
        end
        flags_gt = zeros(n, 1);
    end
    
    % get predicted bounding box
    det = dets{img_idx + 1};
    if isempty(det) == 1
        fprintf('no detection for image %d\n', img_idx);
        continue;
    end
    if max(det(:,6)) < threshold
        fprintf('maximum score %.2f is smaller than threshold\n', max(det(:,6)));
        continue;
    end
    if isempty(det) == 0
        I = nms_new(det, threshold_overlap);
        det = det(I, :);    
    end
    num = size(det, 1);
    
    % for each predicted bounding box
    if is_train
        flags_pr = zeros(num, 1);
        for j = 1:num
            bbox_pr = det(j, 1:4);  

            % compute box overlap
            if isempty(bbox) == 0
                o = boxoverlap(bbox, bbox_pr);
                [maxo, index] = max(o);
                if maxo >= 0.7 && flags_gt(index) == 0
                    flags_pr(j) = 1;
                    flags_gt(index) = 1;
                end
            end
        end
    end
    
    file_img = sprintf('%s/%06d.png', image_dir, img_idx);
    I = imread(file_img);
    
    % show all the detections
    figure(1);
    imshow(I);
    hold on;
    
    for k = 1:size(dets{img_idx + 1},1)
        if dets{img_idx + 1}(k,6) > threshold
            % get predicted bounding box
            bbox_pr = dets{img_idx + 1}(k,1:4);
            bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
        end
    end
    hold off;

    figure(2);
    imshow(I);
    hold on;
    for k = 1:num
        if det(k,6) > threshold
            % get predicted bounding box
            bbox_pr = det(k,1:4);
            bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
            if is_train
                if flags_pr(k)
                    rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
                else
                    rectangle('Position', bbox_draw, 'EdgeColor', 'r', 'LineWidth', 2);
                end
            else
                rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
            end
            text(bbox_pr(1), bbox_pr(2), num2str(det(k,5)), 'FontSize', 16, 'BackgroundColor', 'r');
        end
    end
    
    if is_train
        for k = 1:n
            if flags_gt(k) == 0
                bbox_gt = bbox(k,1:4);
                bbox_draw = [bbox_gt(1), bbox_gt(2), bbox_gt(3)-bbox_gt(1), bbox_gt(4)-bbox_gt(2)];
                rectangle('Position', bbox_draw, 'EdgeColor', 'y', 'LineWidth', 2);
            end
        end
    end
    hold off;
    pause;
end