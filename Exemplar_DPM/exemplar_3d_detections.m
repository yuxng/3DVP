function dets_3d = exemplar_3d_detections

addpath(genpath('../KITTI'));
opt = my_globals();
cls = 'car';
threshold = -1;

% load data
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
tmin = min(data.translation(3,:));
tmax = max(data.translation(3,:));

% load the mean CAD model
filename = sprintf('%s/%s_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
vertices = cad.x3d;

% KITTI path
globals;
root_dir = KITTIroot;
data_set = 'training';
cam = 2;
calib_dir = fullfile(root_dir, [data_set '/calib']);

% load detections
filename = sprintf('kitti_train/%s_test.mat', cls);
object = load(filename);
dets = object.dets;
N = numel(dets);

dets_3d = cell(1,N);
for i = 1:N
    tic;
    det = dets{i};
    
    flag = det(:,6) > threshold;
    det = det(flag,:);
    
    num = size(det, 1);
    T = zeros(3, num);
    fprintf('image %d, %d detections\n', i, num);
    
    % projection matrix
    img_idx = i - 1;
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
        while ry < -pi
            ry = ry + 2*pi;
        end        
        x(4) = ry;
        x(5) = dmean;
        % compute lower bound and upper bound
        lb = [lmin hmin wmin x(4)-15*pi/180 dmin];
        ub = [lmax hmax wmax x(4)+15*pi/180 dmax];
        % optimize
        options = optimset('Algorithm', 'interior-point', 'Display', 'off');
        x = fmincon(@(x)compute_error_distance(x, vertices, C, X, P, Pv2c, width, height),...
            x, [], [], [], [], lb, ub, [], options);

        % compute the translation in camera coordinate
        t = C + x(5).*X;
        T(:,k) = t;
        t(4) = 1;
        t = Pv2c*t;
        
        % compute alpha
        alpha = x(4) - theta;
        while alpha > pi
            alpha = alpha - 2*pi;
        end
        while alpha < -pi
            alpha = alpha + 2*pi;
        end         
        
        % assign variables
        objects(k).alpha = alpha;
        objects(k).l = x(1);
        objects(k).h = x(2);
        objects(k).w = x(3);
        objects(k).ry = x(4);
        objects(k).t = t(1:3)';
        objects(k).T = T(:,k)';
    end
    
    % filter out floating bounding boxes
    flag = T(3,:) >= tmin & T(3,:) <= tmax;
    dets_3d{i} = objects(flag);
    fprintf('%d detections left\n', sum(flag));
    toc;
end

filename = sprintf('kitti_train/%s_test_3d.mat', cls);
save(filename, 'dets_3d', '-v7.3');