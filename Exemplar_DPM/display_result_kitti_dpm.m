function display_result_kitti_dpm

cls = 'car';
vnum = 16;
threshold = -0.9;

% read detection results
filename = sprintf('kitti_train_dpm/%s_pose_%d_test.mat', cls, vnum);
object = load(filename, 'boxes1');
dets = object.boxes1;
fprintf('load detection done\n');

% read ids of validation images
object = load('kitti_ids_new.mat');
ids = object.ids_val;
N = numel(ids);

% KITTI path
globals;
root_dir = KITTIroot;
data_set = 'training';
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

figure;
ind_plot = 1;
for i = 1:N
    img_idx = ids(i);    
    det = dets{i};
    
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
    
    subplot(4, 2, ind_plot);
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
            
%             text(bbox_pr(1), bbox_pr(2), num2str(k), 'FontSize', 16, 'BackgroundColor', 'r');
        end
    end
    subplot(4, 2, ind_plot);
    hold off;
    ind_plot = ind_plot + 1;
    if ind_plot > 8
        ind_plot = 1;
        pause;
    end
end