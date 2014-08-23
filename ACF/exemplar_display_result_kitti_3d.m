function exemplar_display_result_kitti_3d

cls = 'car';
is_train = 0;

% read detection results
if is_train
    filename = sprintf('kitti_train/%s_test_greedy.mat', cls);
else
    filename = sprintf('kitti_test/%s_test_greedy.mat', cls);
end
object = load(filename);
dets = object.dets_greedy;
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

figure;
for i = 1:N
    img_idx = ids(i);
    disp(img_idx);
    det = dets{img_idx + 1};
    
    if isempty(det) == 1
        fprintf('no detection for image %d\n', img_idx);
        continue;
    end
    
    num = numel(det);
    
    file_img = sprintf('%s/%06d.png',image_dir, img_idx);
    I = imread(file_img);
    
    imshow(I);
    hold on;    

    for k = 1:num
        % get predicted bounding box
        bbox_pr = [det(k).x1 det(k).y1 det(k).x2 det(k).y2];
        bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
        rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
        text(bbox_pr(1), bbox_pr(2), num2str(det(k).cid), 'FontSize', 16, 'BackgroundColor', 'r');
    end
    hold off;
    pause;
end