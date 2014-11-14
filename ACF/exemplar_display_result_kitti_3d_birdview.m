function exemplar_display_result_kitti_3d_birdview(dets_all, data)

is_train = 1;
is_save = 1;

addpath(genpath('../KITTI'));
threshold = -17.0673;  % fppi 1.21
cls = 'car';

if is_train
    result_dir = 'kitti_train_ap_125';
else
    result_dir = 'kitti_test_ap_227';
end

% read detection results
if nargin < 1
    result_dir = 'kitti_train_ap_125';
    filename = sprintf('%s/car_3d_aps_125_combined_test_3d.mat', result_dir);
    object = load(filename);
    dets_all = object.dets_3d;
    fprintf('load detection done\n');
end

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
if nargin < 2
    object = load(filename);
    data = object.data;
end

hf = figure;
cmap = colormap(summer);
ind_plot = 1;
mplot = 2;
nplot = 1;
for i = [50, 121, 763, 1859] %1:N
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
    
    if isempty(dets) == 0
        I = dets(:,6) >= threshold;
        dets = dets(I,:);
        objects = objects(I);
        height = dets(:,4) - dets(:,2);
        [~, I] = sort(height);
        dets = dets(I,:);
        objects = objects(I);
    end
    num = size(dets, 1); 
    
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
    subplot(mplot, nplot, ind_plot);
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
%                 [y, x] = ind2sub(size(pattern), index);                
%                 pattern = pattern(min(y):max(y), min(x):max(x));
                
                [y, x] = ind2sub(size(pattern), index);
                cx = size(pattern, 2)/2;
                cy = size(pattern, 1)/2;
                width = size(pattern, 2);
                height = size(pattern, 1);                 
                pattern = pattern(min(y):max(y), min(x):max(x));

                % find the object center
                sx = w / size(pattern, 2);
                sy = h / size(pattern, 1);
                tx = bbox(1) - sx*min(x);
                ty = bbox(2) - sy*min(y);
                cx = sx * cx + tx;
                cy = sy * cy + ty;
                width = sx * width;
                height = sy * height;
                bbox_pr = round([cx-width/2 cy-height/2 cx+width/2 cy+height/2]);
                width = bbox_pr(3) - bbox_pr(1) + 1;
                height = bbox_pr(4) - bbox_pr(2) + 1;
                
                pattern = imresize(data.pattern{cid}, [height width], 'nearest');
                
                bbox = zeros(1,4);
                bbox(1) = max(1, floor(bbox_pr(1)));
                start_x = bbox(1) - floor(bbox_pr(1)) + 1;
                bbox(2) = max(1, floor(bbox_pr(2)));
                start_y = bbox(2) - floor(bbox_pr(2)) + 1;
                bbox(3) = min(size(I,2), floor(bbox_pr(3)));
                bbox(4) = min(size(I,1), floor(bbox_pr(4)));
                w = bbox(3) - bbox(1) + 1;
                h = bbox(4) - bbox(2) + 1;
                pattern = pattern(start_y:start_y+h-1, start_x:start_x+w-1);
            else
                pattern = imresize(pattern, [h w], 'nearest');
            end
            
            % build the pattern in the image
            height = size(I,1);
            width = size(I,2);
            P = uint8(zeros(height, width));
            x = bbox(1);
            y = bbox(2);
            index_y = y:min(y+h-1, height);
            index_x = x:min(x+w-1, width);
            P(index_y, index_x) = pattern(1:numel(index_y), 1:numel(index_x));
            
            % show occluded region
            im = create_occlusion_image(pattern);
            x = bbox(1);
            y = bbox(2);
            Isub = I(y:y+h-1, x:x+w-1, :);
            index = im == 255;
            im(index) = Isub(index);
            I(y:y+h-1, x:x+w-1, :) = uint8(0.1*Isub + 0.9*im);               
            
            % show segments
            index_color = 1 + floor((k-1) * size(cmap,1) / num);
            if is_train
                if flags_pr(k)
                    dispColor = 255*cmap(index_color,:);
                else
                    dispColor = [255 0 0];
                end
            else
                dispColor = 255*cmap(index_color,:);
            end
            scale = round(max(size(I))/400);            
            [gx, gy] = gradient(double(P));
            g = gx.^2 + gy.^2;
            g = conv2(g, ones(scale), 'same');
            edgepix = find(g > 0);
            npix = numel(P);
            for b = 1:3
                I((b-1)*npix+edgepix) = dispColor(b);
            end                       
        end
    end    
    
    imshow(I);
    hold on;

%     if is_train
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
%     end
    
    if is_train
        for k = 1:n
            if flags_gt(k) == 0
                bbox = bbox_gt(k,1:4);
                bbox_draw = [bbox(1), bbox(2), bbox(3)-bbox(1), bbox(4)-bbox(2)];
                % rectangle('Position', bbox_draw, 'EdgeColor', 'b', 'LineWidth', 2, 'LineStyle',':');
                lw = 2;
                rectangle('position', bbox_draw, 'linewidth', lw, 'edgecolor', 'w');
                rectangle('position', bbox_draw, 'linewidth', lw, 'edgecolor', 'b', 'linestyle', ':');                   
            end
        end
    end    
    
    hold off;
    ind_plot = ind_plot + 1;
    
    % plot 3D localization
    subplot(mplot, nplot, ind_plot);
    cla;
    hold on;
    
    sxmin = inf;
    symin = inf;
    sxmax = -inf;
    symax = -inf;
    for k = 1:numel(objects);
        object = objects(k);
        if strcmp(object.type, 'Car') == 1 && object.score >= threshold
            % compute rotational matrix around yaw axis
            R = [+cos(object.ry), 0, +sin(object.ry);
                               0, 1,               0;
                 -sin(object.ry), 0, +cos(object.ry)];

            % 3D bounding box dimensions
            l = object.l;
            w = object.w;
            h = object.h;

            % 3D bounding box corners
            x_corners = [l/2, l/2, -l/2, -l/2, l/2, l/2, -l/2, -l/2];
            y_corners = [0,0,0,0,-h,-h,-h,-h];
            z_corners = [w/2, -w/2, -w/2, w/2, w/2, -w/2, -w/2, w/2];

            % rotate and translate 3D bounding box
            corners_3D = R*[x_corners;y_corners;z_corners];
            corners_3D(1,:) = corners_3D(1,:) + object.t(1);
            corners_3D(2,:) = corners_3D(2,:) + object.t(2);
            corners_3D(3,:) = corners_3D(3,:) + object.t(3);
            
            % transform to world coordinate system
            corners_3D = Pv2c\[corners_3D; ones(1,size(corners_3D,2))];
            
            
            plane_vertex = corners_3D(1:3, 5:8)';
            plane_vertex(:,3) = plane_vertex(:,3) + 1; 
            patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'EdgeColor', [0 1 0],...
                'FaceColor', 'none', 'LineWidth', 2);
            
            
            sxmin = min(sxmin, min(plane_vertex(:,1)));
            symin = min(symin, min(plane_vertex(:,2)));
            sxmax = max(sxmax, max(plane_vertex(:,1)));
            symax = max(symax, max(plane_vertex(:,2)));
        end
    end
    
    % ground truth top view
    if is_train
        objects = readLabels(label_dir,img_idx);
        for k = 1:numel(objects)
            object = objects(k);
            if strcmp(object.type, 'Car') == 1
                % compute rotational matrix around yaw axis
                R = [+cos(object.ry), 0, +sin(object.ry);
                                   0, 1,               0;
                     -sin(object.ry), 0, +cos(object.ry)];

                % 3D bounding box dimensions
                l = object.l;
                w = object.w;
                h = object.h;

                % 3D bounding box corners
                x_corners = [l/2, l/2, -l/2, -l/2, l/2, l/2, -l/2, -l/2];
                y_corners = [0,0,0,0,-h,-h,-h,-h];
                z_corners = [w/2, -w/2, -w/2, w/2, w/2, -w/2, -w/2, w/2];

                % rotate and translate 3D bounding box
                corners_3D = R*[x_corners;y_corners;z_corners];
                corners_3D(1,:) = corners_3D(1,:) + object.t(1);
                corners_3D(2,:) = corners_3D(2,:) + object.t(2);
                corners_3D(3,:) = corners_3D(3,:) + object.t(3);

                % transform to world coordinate system
                corners_3D = Pv2c\[corners_3D; ones(1,size(corners_3D,2))];


                plane_vertex = corners_3D(1:3, 5:8)';
                patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'EdgeColor', [0 0 1], 'FaceColor', 'none', ...
                    'LineWidth', 2);


                sxmin = min(sxmin, min(plane_vertex(:,1)));
                symin = min(symin, min(plane_vertex(:,2)));
                sxmax = max(sxmax, max(plane_vertex(:,1)));
                symax = max(symax, max(plane_vertex(:,2)));
            end
        end
    end    
    

    % draw the camera
    draw_camera(C);

    % draw the ground plane
    h = 1.73;
    sxmin = min(sxmin, C(1)) - 5;
    symin = min(symin, C(2)) - 5;
    sxmax = max(sxmax, C(1)) + 5;
    symax = max(symax, C(2)) + 5;
    plane_vertex = zeros(4,3);
    plane_vertex(1,:) = [sxmin symin -h];
    plane_vertex(2,:) = [sxmax symin -h];
    plane_vertex(3,:) = [sxmax symax -h];
    plane_vertex(4,:) = [sxmin symax -h];
    patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'EdgeColor', [0 0 0], 'FaceColor', 'none', ...
        'LineWidth', 2);
    axis tight;
    axis equal;
    view(-90, 88);
    hold off;
    ind_plot = ind_plot + 1;
    
    if ind_plot > mplot*nplot
        ind_plot = 1;
        if is_save
            if is_train
                filename = fullfile('../', sprintf('%06d_nms.png', img_idx));
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