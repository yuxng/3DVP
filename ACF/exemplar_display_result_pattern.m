function exemplar_display_result_pattern(cid)

cls = 'car';
threshold = -50;
is_train = 0;

% read detection results
filename = sprintf('data/%s_%d_test.mat', cls, cid);
object = load(filename);
dets = object.boxes;

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
            rectangle('Position', bbox_pr, 'EdgeColor', 'g', 'LineWidth',2);
%             text(bbox_pr(1), bbox_pr(2), num2str(k), 'FontSize', 16, 'BackgroundColor', 'r');

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