function exemplar_write_kitti_results_objects

cls = 'car';
is_train = 1;

% read detection results
if is_train == 1
    filename = sprintf('kitti_train/%s_test_greedy.mat', cls);
else
    filename = sprintf('kitti_test/%s_test_greedy.mat', cls);
end
object = load(filename);
dets = object.dets_greedy;
fprintf('load detection done\n');

% read ids of validation images
object = load('kitti_ids.mat');
if is_train == 1
    ids = object.ids_val;
else
    ids = object.ids_test;
end
N = numel(ids);

for i = 1:N
    img_idx = ids(i);    
    objects = dets{img_idx + 1};
    
    % result file
    if is_train == 1
        filename = sprintf('results_kitti_train/%06d.txt', img_idx);
    else
        filename = sprintf('results_kitti_test/%06d.txt', img_idx);
    end
    disp(filename);
    fid = fopen(filename, 'w');
    
    if isempty(objects) == 1
        fprintf('no detection for image %d\n', img_idx);
        fclose(fid);
        continue;
    end
    
    % write detections
    num = numel(objects);
    for k = 1:num
        truncation = objects(k).truncation;
        occlusion = objects(k).occlusion;
        alpha = objects(k).alpha;        
        h = objects(k).h;
        w = objects(k).w;
        l = objects(k).l;
        
        fprintf(fid, '%s %f %d %f %f %f %f %f %f %f %f %f %f %f %f %f\n', ...
            'Car', truncation, occlusion, alpha, objects(k).x1, objects(k).y1, objects(k).x2, objects(k).y2, ...
            h, w, l, -1, -1, -1, -1, objects(k).score);
    end
    fclose(fid);
end