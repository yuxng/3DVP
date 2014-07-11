function display_result_pattern

cls = 'car';
cid = 52;
threshold = -0.95;

% read detection results
filename = sprintf('kitti_train/%s_%d_test.mat', cls, cid);
object = load(filename);
dets = object.boxes1;

% read ids of validation images
object = load('kitti_ids.mat');
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
    det = dets{img_idx + 1};
    
    if isempty(det) == 1
        fprintf('no detection for image %d\n', img_idx);
        continue;
    end
    
    if max(det(:,5)) < threshold
        fprintf('maximum score %.2f is smaller than threshold\n', max(det(:,5)));
        continue;
    end
    num = size(det, 1);
    
    file_img = sprintf('%s/%06d.png',image_dir, img_idx);
    I = imread(file_img);
    
    subplot(4, 2, ind_plot);
    imshow(I);
    hold on;

    til = sprintf('%d', i);
    for k = 1:num
        if det(k,5) > threshold
            % get predicted bounding box
            bbox_pr = det(k,1:4);
            bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth',2);
            text(bbox_pr(1), bbox_pr(2), num2str(k), 'FontSize', 16, 'BackgroundColor', 'r');

            til = sprintf('%s, s%d=%.2f', til, k, det(k,5));
        end
    end
    title(til);
    subplot(4, 2, ind_plot);
    hold off;
    ind_plot = ind_plot + 1;
    if ind_plot > 8
        ind_plot = 1;
        pause;
    end
end