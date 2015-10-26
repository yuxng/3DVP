function exemplar_write_kitti_results_dpm_pedestrian

cls = 'pedestrian';
vnum = 24;
is_train = 1;
azimuth_interval = [0 (360/(vnum*2)):(360/vnum):360-(360/(vnum*2))];

% read detection results
filename = sprintf('data/%s_pose_%d_test.mat', cls, vnum);
object = load(filename);
dets = object.boxes1;
parts = object.parts1;
fprintf('load detection done\n');

% load data
object = load('../KITTI/data_pedestrian.mat');
data = object.data;
centers = unique(data.idx_pose);
centers(centers == -1) = [];

% read ids of validation images
object = load('kitti_ids_new.mat');
if is_train == 1
    ids = object.ids_val;
else
    ids = object.ids_test;
end
N = numel(ids);

for i = 1:N
    img_idx = ids(i);    
    det = dets{i};
    part = parts{i};
    
    % result file
    if is_train == 1
        filename = sprintf('results_kitti_train/%06d.txt', img_idx);
    else
        filename = sprintf('results_kitti_test/%06d.txt', img_idx);
    end
    disp(filename);
    fid = fopen(filename, 'w');
    
    if isempty(det) == 1
        fprintf('no detection for image %d\n', img_idx);
        fclose(fid);
        continue;
    end
    
    % non-maximum suppression
    if isempty(det) == 0
        I = nms(det, 0.5);
        det = det(I, :);
        part = part(I, :);
    end    
    
    % write detections
    num = size(det, 1);
    for k = 1:num
        truncation = 0;
        occlusion = 0;
        
        % azimuth
        cid = centers(part(k,37));
        azimuth_cid = data.azimuth(cid);
        vind = find_interval(azimuth_cid, azimuth_interval);
        azimuth = (vind - 1) * (360 / vnum);
        
        alpha = azimuth + 90;
        if alpha >= 360
            alpha = alpha - 360;
        end
        alpha = alpha*pi/180;
        if alpha > pi
            alpha = alpha - 2*pi;
        end
        
        h = 1;
        w = 1;
        l = 1;
        
        fprintf(fid, '%s %f %d %f %f %f %f %f %f %f %f %f %f %f %f %f\n', ...
            'Pedestrian', truncation, occlusion, alpha, det(k,1), det(k,2), det(k,3), det(k,4), ...
            h, w, l, -1, -1, -1, -1, det(k,6));
    end
    fclose(fid);
end

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