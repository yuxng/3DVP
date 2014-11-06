function exemplar_display_result_kitti_3d

is_train = 1;
is_save = 0;

addpath(genpath('../KITTI'));
threshold = -2;
cls = 'car';
result_dir = 'kitti_train_ap_125';

% read detection results
filename = sprintf('%s/odets_3d.mat', result_dir);
object = load(filename);
dets_all = object.dets_3d;
fprintf('load detection done\n');

% read ids of validation images
object = load('kitti_ids_new.mat');
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end
N = numel(ids);

% load PASCAL3D+ cad models
exemplar_globals;
filename = sprintf(path_cad, cls);
object = load(filename);
cads = object.(cls);
cads([7, 8, 10]) = [];

% KITTI path
root_dir = KITTIroot;
if is_train
    data_set = 'training';
else
    data_set = 'testing';
end
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);
label_dir = fullfile(root_dir,[data_set '/label_' num2str(cam)]);
calib_dir = fullfile(root_dir,[data_set '/calib']);

% load data
if is_train
    filename = fullfile(SLMroot, 'KITTI/data.mat');
else
    filename = fullfile(SLMroot, 'KITTI/data_kitti.mat');
end
object = load(filename);
data = object.data;

hf = figure;
ind_plot = 1;
mplot = 2;
nplot = 2;
for i = 1:N
    disp(i);
    img_idx = ids(i);
    
    % read ground truth bounding box
    if is_train
        objects = readLabels(label_dir, img_idx);
        clsinds = strmatch(cls, lower({objects(:).type}), 'exact');
        n = numel(clsinds);
        bbox_gt = zeros(n, 4); 
        for j = 1:n
            bbox_gt(j,:) = [objects(clsinds(j)).x1 objects(clsinds(j)).y1 ...
                objects(clsinds(j)).x2 objects(clsinds(j)).y2];     
        end
        flags_gt = zeros(n, 1);
    end     
    
    objects = dets_all{i};
    num = numel(objects);
    
    % construct detections
    dets = zeros(num, 6);    
    for k = 1:num
        dets(k,:) = [objects(k).x1 objects(k).y1 objects(k).x2 objects(k).y2 ...
                objects(k).cid objects(k).score];            
    end
    
    if max(dets(:,6)) < threshold
        fprintf('maximum score %.2f is smaller than threshold\n', max(dets(:,6)));
        continue;
    end
    
    % for each predicted bounding box
    if is_train
        flags_pr = zeros(num, 1);
        for j = 1:num
            bbox_pr = dets(j, 1:4);  

            % compute box overlap
            if isempty(bbox_gt) == 0
                o = boxoverlap(bbox_gt, bbox_pr);
                [maxo, index] = max(o);
                if maxo >= 0.7 && flags_gt(index) == 0
                    flags_pr(j) = 1;
                    flags_gt(index) = 1;
                end
            end
        end
    end    
    
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
    subplot(mplot, nplot, [1 2]);
    file_img = sprintf('%s/%06d.png',image_dir, img_idx);
    I = imread(file_img);
    for k = 1:num
        if dets(k,6) > threshold
            bbox_pr = dets(k,1:4);
            bbox = zeros(1,4);
            bbox(1) = max(1, floor(bbox_pr(1)));
            bbox(2) = max(1, floor(bbox_pr(2)));
            bbox(3) = min(size(I,2), floor(bbox_pr(3)));
            bbox(4) = min(size(I,1), floor(bbox_pr(4)));
            w = bbox(3) - bbox(1) + 1;
            h = bbox(4) - bbox(2) + 1;

            % apply the 2D occlusion mask to the bounding box
            % check if truncated pattern
            cid = dets(k,5);
            pattern = data.pattern{cid};                
            index = find(pattern == 1);
            if data.truncation(cid) > 0 && isempty(index) == 0
                [y, x] = ind2sub(size(pattern), index);                
                pattern = pattern(min(y):max(y), min(x):max(x));
            end
            pattern = imresize(pattern, [h w], 'nearest');
            
            % build the pattern in the image
            height = size(I,1);
            width = size(I,2);
            P = uint8(zeros(height, width));
            x = bbox(1);
            y = bbox(2);
            index_y = y:min(y+h-1, height);
            index_x = x:min(x+w-1, width);
            P(index_y, index_x) = pattern(1:numel(index_y), 1:numel(index_x));
            
            % show segments
            if is_train
                if flags_pr(k)
                    dispColor = [0 255 0];
                else
                    dispColor = [255 0 0];
                end
            else
                dispColor = [0 255 0];
            end
            scale = round(max(size(I))/500);            
            [gx, gy] = gradient(double(P));
            g = gx.^2 + gy.^2;
            g = conv2(g, ones(scale), 'same');
            edgepix = find(g > 0);
            npix = numel(P);
            for b = 1:3
                I((b-1)*npix+edgepix) = dispColor(b);
            end            
            
            % show occluded region
            im = create_occlusion_image(pattern);
            x = bbox(1);
            y = bbox(2);
            Isub = I(y:y+h-1, x:x+w-1, :);
            index = im == 255;
            im(index) = Isub(index);
            I(y:y+h-1, x:x+w-1, :) = uint8(0.1*Isub + 0.9*im);              
        end
    end    
    
    imshow(I);
    hold on;

    if is_train
%         for k = 1:num
%             if dets(k,6) > threshold
%                 % get predicted bounding box
%                 bbox = dets(k,1:4);
%                 bbox_draw = [bbox(1), bbox(2), bbox(3)-bbox(1), bbox(4)-bbox(2)];
%                 rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);            
%                 text(bbox(1), bbox(2), num2str(k), 'FontSize', 16, 'BackgroundColor', 'r');
%                 til = sprintf('%s, s%d=%.2f', til, k, objects(k).score);
%                 s = sprintf('%.2f', dets(k,6));
%                 bbox_pr = dets(k,1:4);
%                 text(bbox_pr(1), bbox_pr(2), s, 'FontSize', 4, 'BackgroundColor', 'c');
%             end
%         end
        
        for k = 1:n
            if flags_gt(k) == 0
                bbox = bbox_gt(k,1:4);
                bbox_draw = [bbox(1), bbox(2), bbox(3)-bbox(1), bbox(4)-bbox(2)];
                rectangle('Position', bbox_draw, 'EdgeColor', 'y', 'LineWidth', 2);
            end
        end        
    end
    hold off;
    ind_plot = ind_plot + 2;
    
    % plot 3D localization
    Vpr = [];
    Fpr = [];
    for k = 1:num
        object = objects(k);
        if strcmp(object.type, 'Car') == 1 && object.score > threshold
            cad_index = find_closest_cad(cads, object);
            x3d = compute_3d_points(cads(cad_index).vertices, object);
            face = cads(cad_index).faces;
            tmp = face + size(Vpr,2);
            Fpr = [Fpr; tmp];        
            Vpr = [Vpr x3d];
        end
    end    
    
    if is_train
        subplot(mplot, nplot, ind_plot);
    else
        subplot(mplot, nplot, [3, 4]);
    end
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
        sy = 2*max(abs(Vpr(2,:)));
        c = [mean(Vpr(1:2,:), 2); 0]';
        plane_vertex = zeros(4,3);
        plane_vertex(1,:) = c + [-sx -sy -h];
        plane_vertex(2,:) = c + [sx -sy -h];
        plane_vertex(3,:) = c + [sx sy -h];
        plane_vertex(4,:) = c + [-sx sy -h];
        patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'FaceColor', [0.5 0.5 0.5], 'FaceAlpha', 0.5);

        axis tight;
        % title('3D Estimation');
        view(250, 20);
        hold off;
    end
    if is_train
        ind_plot = ind_plot + 1;
    else
        ind_plot = ind_plot + 2;
    end
    
    % load labels
    if is_train
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
        subplot(mplot, nplot, ind_plot);
        if isempty(Vgt) == 0
            Vgt = Pv2c\[Vgt; ones(1,size(Vgt,2))];
            trimesh(Fgt, Vgt(1,:), Vgt(2,:), Vgt(3,:), 'EdgeColor', 'b');
            hold on;
            axis equal;
%             xlabel('x');
%             ylabel('y');
%             zlabel('z');

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
%             title('3D Ground Truth');
            view(250, 20);
            hold off;
        else
            cla;
        end
        ind_plot = ind_plot + 1;
    end
        
    
    if ind_plot > mplot*nplot
        ind_plot = 1;
        if is_save
            if is_train
                filename = fullfile('result_images_train', sprintf('%06d.png', img_idx));
            else
                filename = fullfile('result_images_test', sprintf('%06d.png', img_idx));
            end
            saveas(hf, filename);
        else
            pause;
        end
    end
end

function im = create_occlusion_image(pattern)

% 2D occlusion mask
im = 255*ones(size(pattern,1), size(pattern,2), 3);
color = [0 0 255];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 2) = color(j);
    im(:,:,j) = tmp;
end
color = [0 255 255];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 3) = color(j);
    im(:,:,j) = tmp;
end
im = uint8(im); 