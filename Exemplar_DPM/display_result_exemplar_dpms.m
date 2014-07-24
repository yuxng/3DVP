function display_result_exemplar_dpms

addpath(genpath('../KITTI'));

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

% load clustering data
object = load('../KITTI/data.mat');
data = object.data;

% compute statistics
lmin = min(data.l);
lmean = mean(data.l);
lmax = max(data.l);
hmin = min(data.h);
hmean = mean(data.h);
hmax = max(data.h);
wmin = min(data.w);
wmean = mean(data.w);
wmax = max(data.w);
dmin = min(data.distance);
dmean = mean(data.distance);
dmax = max(data.distance);

opt = my_globals();

% load the mean CAD model
cls = 'car';
filename = sprintf('%s/%s_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
vertices = cad.x3d;

% load PASCAL3D+ cad models
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
    
    % projection matrix
    P = readCalibration(calib_dir, img_idx, cam);
    
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
    
    Vpr = [];
    Fpr = [];
    
    % apply the 2D occlusion mask to the bounding box
    for k = 1:num
        if det(k,6) > threshold || k <= 5
            % get predicted bounding box
            bbox_pr = det(k,1:4);
            bbox = zeros(1,4);
            bbox(1) = max(1, round(bbox_pr(1)));
            bbox(2) = max(1, round(bbox_pr(2)));
            bbox(3) = min(size(I,2), round(bbox_pr(3)));
            bbox(4) = min(size(I,1), round(bbox_pr(4)));
            w = bbox(3) - bbox(1) + 1;
            h = bbox(4) - bbox(2) + 1;
            
            cid = det(k,5);
            pattern = data.pattern{cid};
            index = find(pattern == 1);            
            % check if truncated pattern
            if data.truncation(cid) > 0 && isempty(index) == 0
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
            else
                cx = (bbox_pr(1) + bbox_pr(3)) / 2;
                cy = (bbox_pr(2) + bbox_pr(4)) / 2;
                width = bbox_pr(3) - bbox_pr(1);
                height = bbox_pr(4) - bbox_pr(2);
            end

            pattern_new = imresize(pattern, [h w], 'nearest');
            im = create_mask_image(pattern_new);
            Isub = I(bbox(2):bbox(4), bbox(1):bbox(3), :);
            index = im == 255;
            im(index) = Isub(index);
            I(bbox(2):bbox(4), bbox(1):bbox(3), :) = uint8(0.1*Isub + 0.9*im);
            
            % backprojection
            c = [cx; cy; 1];
            X = pinv(P) * c;
            X = X ./ X(4);
            if X(3) < 0
                X = -1 * X;
            end
            theta = atan(X(1)/X(3));
            % transform to velodyne space
            X = Pv2c\X;
            X(4) = [];
            % compute the ray
            X = X - C;
            % normalization
            X = X ./ norm(X);     

            % optimization to search for 3D bounding box
            % initialization
            x = zeros(1,5);
            x(1) = lmean;
            x(2) = hmean;
            x(3) = wmean;
            alpha = data.azimuth(cid) + 90;
            ry = alpha*pi/180 + theta;
            while ry > pi
                ry = ry - 2*pi;
            end
            x(4) = ry;
            x(5) = dmean;
            % compute lower bound and upper bound
            lb = [lmin hmin wmin x(4)-15*pi/180 dmin];
            ub = [lmax hmax wmax x(4)+15*pi/180 dmax];
            % optimize
            options = optimset('Algorithm', 'interior-point');
            x = fmincon(@(x)compute_error_distance(x, vertices, C, X, P, Pv2c, width, height),...
                x, [], [], [], [], lb, ub, [], options);
            disp(x);
            
            % compute the translation in camera coordinate
            t = C + x(5).*X;
            t(4) = 1;
            T = Pv2c*t;
            
            % build the object structure
            object.l = x(1);
            object.h = x(2);
            object.w = x(3);
            object.ry = x(4);
            object.t = T(1:3);
            cad_index = find_closest_cad(cads, object);
            x3d = compute_3d_points(cads(cad_index).vertices, object);
            face = cads(cad_index).faces;
        
            tmp = face + size(Vpr,2);
            Fpr = [Fpr; tmp];        
            Vpr = [Vpr x3d];         
        end
    end    
    
    % plot 2D detections
    subplot(3, 3, ind_plot);
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
    subplot(3, 3, ind_plot);
    hold off;
    ind_plot = ind_plot + 1;
    
    % plot 3D localization
    subplot(3, 3, ind_plot);
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
        s = max(max(abs(Vpr(1:2,:))));
        c = [mean(Vpr(1:2,:), 2); 0]';
        plane_vertex = zeros(4,3);
        plane_vertex(1,:) = c + [-s -s -h];
        plane_vertex(2,:) = c + [s -s -h];
        plane_vertex(3,:) = c + [s s -h];
        plane_vertex(4,:) = c + [-s s -h];
        patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'FaceColor', [0.5 0.5 0.5], 'FaceAlpha', 0.5);

        axis tight;
        title('3D Estimation');
        view(30, 30);
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
    subplot(3, 3, ind_plot);
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
        s = max(max(abs(Vgt(1:2,:))));
        c = [mean(Vgt(1:2,:), 2); 0]';
        plane_vertex = zeros(4,3);
        plane_vertex(1,:) = c + [-s -s -h];
        plane_vertex(2,:) = c + [s -s -h];
        plane_vertex(3,:) = c + [s s -h];
        plane_vertex(4,:) = c + [-s s -h];
        patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'FaceColor', [0.5 0.5 0.5], 'FaceAlpha', 0.5);

        axis tight;
        title('3D Ground Truth');
        view(30, 30);
        hold off;
    else
        cla;
    end
    ind_plot = ind_plot + 1;    
    
    if ind_plot > 9
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