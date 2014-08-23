function exemplar_display_result_kitti

cls = 'car';
threshold = -50;
is_train = 0;

% read detection results
if is_train
    filename = sprintf('kitti_train/%s_test.mat', cls);
else
    filename = sprintf('kitti_test/%s_test.mat', cls);
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

figure;
for i = 1:N
    img_idx = ids(i);
    disp(img_idx);
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
        I = nms(det, 0.5);
        det = det(I, :);    
    end    
    
    num = size(det, 1);
    
    file_img = sprintf('%s/%06d.png',image_dir, img_idx);
    I = imread(file_img);
    
    imshow(I);
    hold on;    

    for k = 1:num
        if det(k,6) > threshold
            % get predicted bounding box
            bbox_pr = det(k,1:4);
            bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
%             x = [bbox_pr(1) bbox_pr(3) bbox_pr(3) bbox_pr(1)];
%             y = [bbox_pr(2) bbox_pr(2) bbox_pr(4) bbox_pr(4)];
%             patch(x, y, 'g', 'FaceAlpha', 0.1, 'EdgeAlpha', 0);
            
            text(bbox_pr(1), bbox_pr(2), num2str(det(k,5)), 'FontSize', 16, 'BackgroundColor', 'r');
        end
    end
    hold off;
    pause;
end