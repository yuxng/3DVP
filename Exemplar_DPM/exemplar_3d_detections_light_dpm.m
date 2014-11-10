function exemplar_3d_detections_light_dpm

matlabpool open;

addpath(genpath('../KITTI'));
cls = 'car';
threshold = -inf;
threshold_nms = 0.5;

vnum = 16;
is_train = 1;
azimuth_interval = [0 (360/(vnum*2)):(360/vnum):360-(360/(vnum*2))];
result_dir = 'kitti_train_dpm';
name = 'pose_16_test_09';

% load data
if is_train
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_kitti.mat');
end
data = object.data;
centers = unique(data.idx_pose);
centers(centers == -1) = [];

% compute statistics
lmean = mean(data.l);
hmean = mean(data.h);
wmean = mean(data.w);
dmin = min(data.distance);
dmean = mean(data.distance);
dmax = max(data.distance);

% load the mean CAD model
filename = sprintf('../Geometry/%s_mean.mat', cls);
object = load(filename);
cad = object.(cls);
vertices = cad.x3d;

% KITTI path
globals;
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
filename = sprintf('%s/%s_%s.mat', result_dir, cls, name);
object = load(filename);
dets = object.boxes1;
parts = object.parts1;
dets_3d = cell(1, N);

parfor i = 1:N
    img_idx = ids(i);
    tic;
    det = dets{i};
    part = parts{i};
    
    % thresholding and nms
    if isempty(det) == 0
        flag = det(:,6) > threshold;
        det = det(flag,:);
        I = nms(det, threshold_nms);
        det = det(I, :);
        part = part(I, :);
    end     
    
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
        cid = centers(part(k,37));
        
        objects(k).type = 'Car';
        objects(k).x1 = bbox(1);
        objects(k).y1 = bbox(2);
        objects(k).x2 = bbox(3);
        objects(k).y2 = bbox(4);        
        objects(k).cid = cid;
        objects(k).score = det(k,6);
        % apply the 2D occlusion mask to the bounding box
        pattern = data.pattern{cid};
        cx = (bbox(1) + bbox(3)) / 2;
        cy = (bbox(2) + bbox(4)) / 2;
        width = w;
        height = h;
        objects(k).truncation = 0;
        objects(k).occlusion = 0;
        objects(k).occ_per = 0;
        objects(k).pattern = imresize(pattern, [h w], 'nearest');

        % backprojection
        c = [cx; cy + height/2; 1];
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
        azimuth_cid = data.azimuth(cid);
        vind = find_interval(azimuth_cid, azimuth_interval);
        azimuth = (vind - 1) * (360 / vnum);        
        
        alpha = azimuth + 90;
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
        x = fmincon(@(x)compute_error(x, x3d, C, X, P, Pv2c, width, height, hmean),...
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
    
    dets_3d{i} = objects;
    
    toc;
end

filename = sprintf('%s/%s_%s_3d.mat', result_dir, cls, name);
save(filename, 'dets_3d', '-v7.3');

matlabpool close;


% compute the projection error between 3D bbox and 2D bbox
function error = compute_error(x, x3d, C, X, P, Pv2c, bw, bh, h)

% compute the translate of the 3D bounding box
t = C + x .* X;
t = Pv2c*[t; 1];
t(4) = [];

% compute 3D points
x3d(1,:) = x3d(1,:) + t(1);
x3d(2,:) = x3d(2,:) + t(2) - h/2;
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
x3d(2,:) = x3d(2,:) + t(2);
x3d(3,:) = x3d(3,:) + t(3);


function ind = find_interval(azimuth, a)

for i = 1:numel(a)
    if azimuth < a(i)
        break;
    end
end
ind = i - 1;
if azimuth > a(end)
    ind = 1;
end