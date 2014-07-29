function display_result_3d(dets_3d)

addpath(genpath('../KITTI'));
threshold = -inf;

% read ids of validation images
object = load('kitti_ids.mat');
ids = object.ids_val;
N = numel(ids);

% load PASCAL3D+ cad models
cls = 'car';
opt = my_globals();
filename = sprintf(opt.path_cad, cls);
object = load(filename);
cads = object.(cls);
cads([7, 8, 10]) = [];

% KITTI path
globals;
root_dir = KITTIroot;
data_set = 'training';
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);
label_dir = fullfile(root_dir,[data_set '/label_' num2str(cam)]);
calib_dir = fullfile(root_dir,[data_set '/calib']);

figure;
ind_plot = 1;
for i = 1:N
    img_idx = ids(i);    
    objects = dets_3d{img_idx + 1};
    num = numel(objects);
    
    % construct detections
    dets = zeros(num, 6);    
    for k = 1:num
        dets(k,:) = [objects(k).x1 objects(k).y1 objects(k).x2 objects(k).y2 ...
                k objects(k).score];            
    end
    if isempty(dets) == 0
        I = nms(dets, 0.5);
        dets = dets(I, :);    
    end
    num = size(dets, 1);
    
    % load the velo_to_cam matrix
    R0_rect = readCalibration(calib_dir, img_idx, 4);
    tmp = R0_rect';
    tmp = tmp(1:9);
    tmp = reshape(tmp, 3, 3);
    tmp = tmp';
    Pv2c = readCalibration(calib_dir, img_idx, 5);
    Pv2c = tmp * Pv2c;
    Pv2c = [Pv2c; 0 0 0 1];
    
    % camera location in world
    C = Pv2c\[0; 0; 0; 1];
    C(4) = [];    
    
    % plot 2D detections
    subplot(2, 2, [1 2]);
    file_img = sprintf('%s/%06d.png',image_dir, img_idx);
    I = imread(file_img);
    for k = 1:num
        if dets(k,6) > threshold
            % get predicted bounding box
            bbox = dets(k,1:4);
            ind = dets(k,5);
            im = create_mask_image(objects(ind).pattern);
            h = size(im, 1);
            w = size(im, 2);
            x = max(1, floor(bbox(1)));
            y = max(1, floor(bbox(2)));
            Isub = I(y:y+h-1, x:x+w-1, :);
            index = im == 255;
            im(index) = Isub(index);
            I(y:y+h-1, x:x+w-1, :) = uint8(0.1*Isub + 0.9*im);
        end
    end    
    
    imshow(I);
    hold on;

    til = sprintf('%d', i);
    for k = 1:num
        if dets(k,6) > threshold
            % get predicted bounding box
            bbox = dets(k,1:4);
            bbox_draw = [bbox(1), bbox(2), bbox(3)-bbox(1), bbox(4)-bbox(2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);            
            text(bbox(1), bbox(2), num2str(k), 'FontSize', 16, 'BackgroundColor', 'r');
            til = sprintf('%s, s%d=%.2f', til, k, objects(k).score);
        end
    end
    title(til);
    hold off;
    ind_plot = ind_plot + 2;
    
    % plot 3D localization
    Vpr = [];
    Fpr = [];
    for k = 1:num
        ind = dets(k,5);
        object = objects(ind);
        if strcmp(object.type, 'Car') == 1 && object.score > threshold
            cad_index = find_closest_cad(cads, object);
            x3d = compute_3d_points(cads(cad_index).vertices, object);
            face = cads(cad_index).faces;
            tmp = face + size(Vpr,2);
            Fpr = [Fpr; tmp];        
            Vpr = [Vpr x3d];
        end
    end    
    
    subplot(2, 2, ind_plot);
    if isempty(Vpr) == 0
        Vpr = Pv2c\[Vpr; ones(1,size(Vpr,2))];
        trimesh(Fpr, Vpr(1,:), Vpr(2,:), Vpr(3,:), 'EdgeColor', 'b');
        hold on;
        axis equal;
        xlabel('x');
        ylabel('y');
        zlabel('z');

        % draw the camera
        draw_camera(C);

        % draw the ground plane
        h = 1.73;
        sx = max(abs(Vpr(1,:)));
        sy = 3*max(abs(Vpr(2,:)));
        c = [mean(Vpr(1:2,:), 2); 0]';
        plane_vertex = zeros(4,3);
        plane_vertex(1,:) = c + [-sx -sy -h];
        plane_vertex(2,:) = c + [sx -sy -h];
        plane_vertex(3,:) = c + [sx sy -h];
        plane_vertex(4,:) = c + [-sx sy -h];
        patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'FaceColor', [0.5 0.5 0.5], 'FaceAlpha', 0.5);

        axis tight;
        title('3D Estimation');
        view(250, 20);
        hold off;
    end
    ind_plot = ind_plot + 1;
    
    % load labels
    objects = readLabels(label_dir,img_idx);
    Vgt = [];
    Fgt = [];
    for k = 1:numel(objects)
        object = objects(k);
        if strcmp(object.type, 'Car') == 1
            cad_index = find_closest_cad(cads, object);
            x3d = compute_3d_points(cads(cad_index).vertices, object);
            face = cads(cad_index).faces;
            tmp = face + size(Vgt,2);
            Fgt = [Fgt; tmp];        
            Vgt = [Vgt x3d];
        end
    end
    
    % plot 3D ground truth
    subplot(2, 2, ind_plot);
    if isempty(Vgt) == 0
        Vgt = Pv2c\[Vgt; ones(1,size(Vgt,2))];
        trimesh(Fgt, Vgt(1,:), Vgt(2,:), Vgt(3,:), 'EdgeColor', 'b');
        hold on;
        axis equal;
        xlabel('x');
        ylabel('y');
        zlabel('z');

        % draw the camera
        draw_camera(C);

        % draw the ground plane
        h = 1.73;
        sx = max(abs(Vgt(1,:)));
        sy = 3*max(abs(Vgt(2,:)));
        c = [mean(Vgt(1:2,:), 2); 0]';
        plane_vertex = zeros(4,3);
        plane_vertex(1,:) = c + [-sx -sy -h];
        plane_vertex(2,:) = c + [sx -sy -h];
        plane_vertex(3,:) = c + [sx sy -h];
        plane_vertex(4,:) = c + [-sx sy -h];
        patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'FaceColor', [0.5 0.5 0.5], 'FaceAlpha', 0.5);

        axis tight;
        title('3D Ground Truth');
        view(250, 20);
        hold off;
    else
        cla;
    end
    ind_plot = ind_plot + 1;    
    
    if ind_plot > 4
        ind_plot = 1;
        pause;
    end
end

function im = create_mask_image(pattern)

% 2D occlusion mask
im = 255*ones(size(pattern,1), size(pattern,2), 3);
color = [0 255 0];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 1) = color(j);
    im(:,:,j) = tmp;
end
color = [255 0 0];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 2) = color(j);
    im(:,:,j) = tmp;
end
im = uint8(im);  