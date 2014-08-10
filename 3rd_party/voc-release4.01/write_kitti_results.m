function write_kitti_results

cls = 'car';

% read detection results
filename = sprintf('data/%s_test_16.mat', cls);
object = load(filename);
dets = object.dets;
fprintf('load detection done\n');

% read ids of validation images
object = load('kitti_ids.mat');
ids = object.ids_val;
N = numel(ids);

for i = 1:N
    img_idx = ids(i);    
    det = dets{i};
    
    % result file
    filename = sprintf('../../Exemplar_DPM/results_kitti_dpm/%06d.txt', img_idx);
    disp(filename);
    fid = fopen(filename, 'w');
    
    if isempty(det) == 1
        fprintf('no detection for image %d\n', img_idx);
        fclose(fid);
        continue;
    end 
    
    % write detections
    num = size(det, 1);
    for k = 1:num
        truncation = -1;
        occlusion = -1;
        alpha = -1;
        h = -1;
        w = -1;
        l = -1;
        
        fprintf(fid, '%s %f %d %f %f %f %f %f %f %f %f %f %f %f %f %f\n', ...
            'Car', truncation, occlusion, alpha, det(k,1), det(k,2), det(k,3), det(k,4), ...
            h, w, l, -1, -1, -1, -1, det(k,6));
    end
    fclose(fid);
end