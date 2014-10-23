function exemplar_cluster_detections

cls = 'car';
is_train = 1;
overlap = 0.6;

% KITTI path
globals;
root_dir = KITTIroot;
if is_train
    data_set = 'training';
else
    data_set = 'testing';
end
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% read ids of validation images
object = load('kitti_ids_new.mat');
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end
N = numel(ids);

% load detections
filename = sprintf('../ACF/kitti_train_ap_125/%s_3d_aps_125_combined_test.mat', cls);
object = load(filename);
dets = object.dets;
cmap = colormap;

for i = 1:N
    det = dets{i};
    idx = nms_clustering(det, overlap);
    
    % read image
    file_img = sprintf('%s/%06d.png', image_dir, ids(i));
    Iimage = imread(file_img);
    
    figure(1);
    imshow(Iimage);
    hold on;
    centers = unique(idx);
    n = numel(centers);
    fprintf('%06d, %d detections, %d clusters\n', ids(i), size(det,1), n);
    for j = 1:n
        index = find(idx == centers(j));
        for k = 1:numel(index)
            bbox = det(index(k), 1:4);
            bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            cindex = round(j * size(cmap,1) / n);
            rectangle('Position', bbox_draw', 'EdgeColor', cmap(cindex,:));
            text(bbox(1), bbox(2), num2str(j), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
        end
    end
    hold off;
    
    figure(2);
    imshow(Iimage);
    hold on;
    for j = 1:n
        bbox = det(centers(j), 1:4);
        bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
        cindex = round(j * size(cmap,1) / n);
        rectangle('Position', bbox_draw', 'EdgeColor', cmap(cindex,:), 'LineWidth', 2);
        text(bbox(1), bbox(2), num2str(j), 'BackgroundColor', [.7 .9 .7], 'Color', 'r');
    end
    hold off;
    
    pause;
end