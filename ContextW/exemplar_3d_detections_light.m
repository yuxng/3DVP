function exemplar_3d_detections_light

matlabpool open;

addpath(genpath('../KITTI'));
cls = 'car';
is_train = 1;
result_dir = 'kitti_train_ap_125';

% load data
if is_train
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_kitti.mat');
end
data = object.data;

% compute statistics
lmean = mean(data.l);
hmean = mean(data.h);
wmean = mean(data.w);
dmin = min(data.distance);
dmean = mean(data.distance);
dmax = max(data.distance);
tmin = min(data.translation(3,:));
tmax = max(data.translation(3,:));

% load the mean CAD model
filename = sprintf('../Geometry/%s_mean.mat', cls);
object = load(filename);
cad = object.(cls);
vertices = cad.x3d;

% KITTI path
exemplar_globals;
root_dir = KITTIroot;
if is_train
    data_set = 'training';
else
    data_set = 'testing';
end
cam = 2;
calib_dir = fullfile(root_dir, [data_set '/calib']);

% read ids of validation images
object = load('kitti_ids_new.mat');
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end
N = numel(ids);

% load detections
filename = sprintf('%s/odets.mat', result_dir);
object = load(filename);
dets = object.odets;
dets_3d = cell(1, N);

parfor i = 1:N
    img_idx = ids(i);
    tic;
    det = dets{i};
    
    num = size(det, 1);
    T = zeros(3, num);
    fprintf('image %d, %d detections\n', i, num);
    
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
    
    % backproject each detection into 3D
    objects = [];
    for k = 1:num
        % get predicted bounding box
        bbox = det(k,1:4);
        w = bbox(3) - bbox(1) + 1;
        h = bbox(4) - bbox(2) + 1;
        cid = det(k,5);
        
        objects(k).type = 'Car';
        objects(k).x1 = bbox(1);
        objects(k).y1 = bbox(2);
        objects(k).x2 = bbox(3);
        objects(k).y2 = bbox(4);        
        objects(k).cid = cid;
        objects(k).score = det(k,6);
        % apply the 2D occlusion mask to the bounding box
        % check if truncated pattern
        pattern = data.pattern{cid};                
        index = find(pattern == 1);
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
            objects(k).truncation = data.truncation(cid);
            objects(k).occlusion = 0;
        else
            cx = (bbox(1) + bbox(3)) / 2;
            cy = (bbox(2) + bbox(4)) / 2;
            width = w;
            height = h;
            objects(k).truncation = 0;
            occ_per = data.occ_per(cid);
            if occ_per > 0.5
                objects(k).occlusion = 2;
            elseif occ_per > 0
                objects(k).occlusion = 1;
            else
                objects(k).occlusion = 0;
            end
        end
        objects(k).occ_per = data.occ_per(cid);
        objects(k).pattern = imresize(pattern, [h w], 'nearest');

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
        % compute 3D points without translation
        alpha = data.azimuth(cid) + 90;
        ry = alpha*pi/180 + theta;
        while ry > pi
            ry = ry - 2*pi;
        end
        while ry < -pi
            ry = ry + 2*pi;
        end
        x3d = compute_3d_points(vertices, lmean, hmean, wmean, ry, [0; 0; 0]);
        
        % initialization
        x = dmean;
        % compute lower bound and upper bound
        lb = dmin;
        ub = dmax;
        % optimize
        options = optimset('Algorithm', 'interior-point', 'Display', 'off');
        x = fmincon(@(x)compute_error(x, x3d, C, X, P, Pv2c, width, height),...
            x, [], [], [], [], lb, ub, [], options);

        % compute the translation in camera coordinate
        t = C + x.*X;
        T(:,k) = t;
        t(4) = 1;
        t = Pv2c*t;
        
        % compute alpha
        alpha = alpha*pi/180;
        while alpha > pi
            alpha = alpha - 2*pi;
        end
        while alpha < -pi
            alpha = alpha + 2*pi;
        end         
        
        % assign variables
        objects(k).alpha = alpha;
        objects(k).l = lmean;
        objects(k).h = hmean;
        objects(k).w = wmean;
        objects(k).ry = ry;
        objects(k).t = t(1:3)';
        objects(k).T = T(:,k)';
    end
    
    % filter out floating bounding boxes
    % flag = T(3,:) >= tmin & T(3,:) <= tmax;
    % objects = objects(flag);
    
    dets_3d{i} = objects;    
    
    % fprintf('%d detections left\n', sum(flag));
    toc;
end

filename = sprintf('%s/odets_3d.mat', result_dir);
save(filename, 'dets_3d', '-v7.3');

matlabpool close;


% compute the projection error between 3D bbox and 2D bbox
function error = compute_error(x, x3d, C, X, P, Pv2c, bw, bh)

% compute the translate of the 3D bounding box
t = C + x .* X;
t = Pv2c*[t; 1];
t(4) = [];

% compute 3D points
x3d(1,:) = x3d(1,:) + t(1);
x3d(2,:) = x3d(2,:) + t(2);
x3d(3,:) = x3d(3,:) + t(3);

% project the 3D bounding box into the image plane
x2d = projectToImage(x3d, P);

% compute bounding box width and height
width = max(x2d(1,:)) - min(x2d(1,:));
height = max(x2d(2,:)) - min(x2d(2,:));

% compute error
error = (width - bw)^2 + (height - bh)^2;


function x3d = compute_3d_points(vertices, l, h, w, ry, t)

x3d = vertices';

% rotation matrix to transform coordinate systems
Rx = [1 0 0; 0 0 -1; 0 1 0];
Ry = [cos(-pi/2) 0 sin(-pi/2); 0 1 0; -sin(-pi/2) 0 cos(-pi/2)];
x3d = Ry*Rx*x3d;

% scaling factors
sx = l / (max(x3d(1,:)) - min(x3d(1,:)));
sy = h / (max(x3d(2,:)) - min(x3d(2,:)));
sz = w / (max(x3d(3,:)) - min(x3d(3,:)));
x3d = diag([sx sy sz]) * x3d;

% compute rotational matrix around yaw axis
R = [+cos(ry), 0, +sin(ry);
        0, 1,       0;
     -sin(ry), 0, +cos(ry)];

% rotate and translate 3D bounding box
x3d = R*x3d;
x3d(1,:) = x3d(1,:) + t(1);
x3d(2,:) = x3d(2,:) + t(2) - h/2;
x3d(3,:) = x3d(3,:) + t(3);