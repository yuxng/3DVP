function exemplar_write_kitti_results

cls = 'car';

% read detection results
filename = sprintf('kitti_test/%s_test.mat', cls);
object = load(filename);
dets = object.dets;
fprintf('load detection done\n');

% load clustering data
object = load('../KITTI/data_trainval.mat');
data = object.data;

% read ids of validation images
object = load('kitti_ids.mat');
ids = object.ids_test;
N = numel(ids);

for i = 1:N
    img_idx = ids(i);    
    det = dets{img_idx + 1};
    
    % result file
    filename = sprintf('results_kitti/%06d.txt', img_idx);
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
    end    
    
    % write detections
    num = size(det, 1);
    for k = 1:num
        cid = det(k, 5);
        truncation = data.truncation(cid);
        
        occ_per = data.occ_per(cid);
        if occ_per > 0.5
            occlusion = 2;
        elseif occ_per > 0
            occlusion = 1;
        else
            occlusion = 0;
        end
        
        azimuth = data.azimuth(cid);
        alpha = azimuth + 90;
        if alpha >= 360
            alpha = alpha - 360;
        end
        alpha = alpha*pi/180;
        if alpha > pi
            alpha = alpha - 2*pi;
        end
        
        h = data.h(cid);
        w = data.w(cid);
        l = data.l(cid);
        
        fprintf(fid, '%s %f %d %f %f %f %f %f %f %f %f %f %f %f %f %f\n', ...
            'Car', truncation, occlusion, alpha, det(k,1), det(k,2), det(k,3), det(k,4), ...
            h, w, l, -1, -1, -1, -1, det(k,6));
    end
    fclose(fid);
end