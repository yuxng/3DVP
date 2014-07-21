function display_result_exemplar_dpms

cls = 'car';
threshold = -0.9;

% read detection results
filename = sprintf('kitti_train/%s_test.mat', cls);
object = load(filename);
dets = object.dets;
fprintf('load detection done\n');

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
    else
        I = nms(det, 0.5);
        det = det(I, :);         
    end
    num = size(det, 1);
    
    file_img = sprintf('%s/%06d.png',image_dir, img_idx);
    I = imread(file_img);
    
    subplot(4, 2, ind_plot);
    imshow(I);
    hold on;

    til = sprintf('%d', i);
    for k = 1:num
        if det(k,6) > threshold || k <= 5
            % get predicted bounding box
            bbox_pr = det(k,1:4);
            bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);            
            text(bbox_pr(1), bbox_pr(2), num2str(k), 'FontSize', 16, 'BackgroundColor', 'r');
            til = sprintf('%s, s%d=%.2f', til, k, det(k,6));   
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