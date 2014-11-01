function exemplar_write_kitti_results_objects

is_train = 1;
result_dir = 'kitti_train_ap_125';
imsize = [1224, 370]; % kittisize

% read detection results
filename = sprintf('%s/odets_3d.mat', result_dir);
object = load(filename);
dets = object.dets_3d;
fprintf('load detection done\n');

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
    objects = dets{i};
    
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
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % truncated..
        if objects(k).x1 < 0
            objects(k).x1 = 0;
        elseif objects(k).x1 > imsize(1)
            objects(k).x1 = imsize(1);
        end
        
        if objects(k).x2 < 0
            objects(k).x2 = 0;
        elseif objects(k).x2 > imsize(1)
            objects(k).x2 = imsize(1);
        end        
        
        if objects(k).y1 < 0
            objects(k).y1 = 0;
        elseif objects(k).y1 > imsize(2)
            objects(k).y1 = imsize(2);
        end
        
        if objects(k).y2 < 0
            objects(k).y2 = 0;
        elseif objects(k).y2 > imsize(2)
            objects(k).y2 = imsize(2);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        
        
        fprintf(fid, '%s %f %d %f %f %f %f %f %f %f %f %f %f %f %f %f\n', ...
            'Car', truncation, occlusion, alpha, objects(k).x1, objects(k).y1, objects(k).x2, objects(k).y2, ...
            h, w, l, -1, -1, -1, -1, objects(k).score);
    end
    fclose(fid);
end