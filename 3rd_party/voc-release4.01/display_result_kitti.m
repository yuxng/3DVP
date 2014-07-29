function display_result_kitti

cls = 'car';
threshold = 0;

% read detection results
filename = sprintf('data/%s_test.mat', cls);
object = load(filename);
dets = object.dets;

% read ids of validation images
object = load('kitti_ids.mat');
ids = object.ids_val;
N = numel(ids);

% KITTI path
globals;
root_dir = KITTIroot;
data_set = 'training';
cam = 2;
image_dir = fullfile(root_dir,[data_set '/image_' num2str(cam)]);

figure;
for i = 1:N
    det = dets{i};
    num = size(det, 1);
    
    img_idx = ids(i);
    file_img = sprintf('%s/%06d.png',image_dir, img_idx);
    I = imread(file_img);
    
    if i ~= 1 && mod(i-1, 8) == 0
        pause;
    end
    ind = mod(i-1,8)+1;
    
    subplot(4, 2, ind);
    imshow(I);
    hold on;

    til = sprintf('%d', i);
    for k = 1:num
        if det(k,6) > threshold || k <= 5
            % get predicted bounding box
            bbox_pr = det(k,1:4);
            bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth',2);
            text(bbox_pr(1), bbox_pr(2), num2str(k), 'FontSize', 8, 'BackgroundColor', 'r');
            til = sprintf('%s, s%d=%.2f', til, k, det(k,6));
        end
    end
    title(til);
    subplot(4, 2, ind);
    hold off;
end