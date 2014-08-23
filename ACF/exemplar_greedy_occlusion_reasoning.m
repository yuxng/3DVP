function dets_greedy = exemplar_greedy_occlusion_reasoning

matlabpool open;
is_train = 0;

addpath(genpath('../KITTI'));

% KITTI path
exemplar_globals;
root_dir = KITTIroot;
if is_train
    data_set = 'training';
else
    data_set = 'testing';
end
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% read ids of validation images
object = load('kitti_ids.mat');
if is_train
    ids = [object.ids_train object.ids_val];
else
    ids = object.ids_test;
end
M = numel(ids);
dets_greedy = cell(1, M);

parfor id = 1:M
    % read image
    fprintf('%d\n', ids(id));
    file_img = sprintf('%s/%06d.png',image_dir, ids(id));
    I = imread(file_img);  
    width = size(I, 2);
    height = size(I, 1);

    % get detection results
    if is_train
        filename = sprintf('results_3d_train/%06d_3d.mat', ids(id));
    else
        filename = sprintf('results_3d_test/%06d_3d.mat', ids(id));
    end
    record = load(filename);
    objects = record.objects;
    num = numel(objects);
    
    if num == 0
        continue;
    end
    
    scores = zeros(1, num);
    distances = zeros(1, num);
    patterns = uint8(zeros(height, width, num));
    dets = zeros(num, 6);    
    for i = 1:num
        scores(i) = objects(i).score;
        distances(i) = norm(objects(i).t);
        pattern = objects(i).pattern;
        h = size(pattern, 1);
        w = size(pattern, 2);
        x = max(1, floor(objects(i).x1));
        y = max(1, floor(objects(i).y1));
        index_y = y:min(y+h-1,height);
        index_x = x:min(x+w-1,width);
        patterns(index_y, index_x, i) = pattern(1:numel(index_y), 1:numel(index_x));
        dets(i,:) = [objects(i).x1 objects(i).y1 objects(i).x2 objects(i).y2 ...
                i objects(i).score];            
    end
    
    if num == 1
        idx = 1;
    else
        % AP clustering on bounding boxes
        N = num*num-num;
        s = zeros(N,3); % Make ALL N^2-N similarities
        j = 1;
        for i = 1:num
            for k = [1:i-1,i+1:num]
                s(j,1) = i;
                s(j,2) = k;
                s(j,3) = boxoverlap(dets(i,1:4), dets(k,1:4));
                j = j+1;
            end
        end
        p = mean(3*s(:,3));
        idx = apclustermex(s, p);
    end
    
    % sort clusters
    cid = unique(idx);
    n = numel(cid);
    scores_cluster = zeros(1, n);
    for i = 1:n
        scores_cluster(i) = max(scores(idx == cid(i)));
    end
    [~, index] = sort(scores_cluster, 'descend');
    cid = cid(index);

    % add clusters into the scene greedily
    flags = zeros(1, n);
    for i = 1:n
        di = distances(idx == cid(i));
        pi = patterns(:, :, idx == cid(i));
        flags(i) = 1;
        for j = 1:i-1
            % for each objects in the scene
            if flags(j) == 1
                dj = distances(idx == cid(j));
                pj = patterns(:, :, idx == cid(j));
                
                % compute the matching score of two sets of occlusion patterns
                smax = -inf;
                for ii = 1:size(pi, 3)
                    dii = di(ii);
                    pii = pi(:,:,ii);
                    for jj = 1:size(pj, 3)
                        djj = dj(jj);
                        pjj = pj(:,:,jj);
                        index = pii > 0 & pjj > 0;
                        overlap = sum(sum(index));
                        ri = overlap / sum(sum(pii > 0));
                        rj = overlap / sum(sum(pjj > 0));
                        if dii > djj  % object i is occluded
                            if ri < 0.1
                                s = 1;
                            else
                                s = sum(pii(index) == 2) / overlap;
                            end
                        else  % object j is occluded
                            if rj < 0.1
                                s = 1;
                            else
                                s = sum(pjj(index) == 2) / overlap;
                            end
                        end
                        smax = max(smax, s);
                    end
                end
                if smax < 0.5  % incompatitable
                    flags(i) = 0;
                    break;
                end
            end
        end
    end
    
    % create the occlusion mask of all the objects
    % sort objects from large distance to small distance
    c = cid(flags == 1);
    nc = numel(c);
    dis = zeros(1, nc);
    index_cluster = zeros(1, nc);
    for i = 1:nc
        index = find(idx == c(i));
        [~, ind] = max(scores(index));
        index_cluster(i) = index(ind);
        dis(i) = distances(index(ind));
    end
    [~, index] = sort(dis, 'descend');

    c = c(index);
    index_cluster = index_cluster(index);
    mask = uint8(zeros(height, width));
    for i = 1:nc
        pattern = patterns(:,:,index_cluster(i));
        mask(pattern > 0) = i;
    end
    
    % check the compatibility of the whole scene for occluded regions
    for i = 1:nc
        pattern = patterns(:, :, idx == c(i));
        smax = -inf;
        for j = 1:size(pattern, 3)
            intersection = pattern(:,:,j) == 2 & mask ~= i;
            occluded = pattern(:,:,j) == 2;
            r = sum(sum(occluded)) / sum(sum(pattern(:,:,j) > 0));
            if r < 0.1
                s = 1;
            else
                s = sum(sum(intersection)) / sum(sum(occluded));
            end
            smax = max(smax, s);
        end
        if smax < 0.8
            flags(cid == c(i)) = 0;
        end
    end
    
    % compute the flags for objects
    flags_cluster = flags;
    flags = zeros(1, num);
    for i = 1:n
        if flags_cluster(i) == 1
            index = find(idx == cid(i));
            [~, ind] = max(scores(index));
            flags(index(ind)) = 1;
        end
    end

    dets_greedy{id} = objects(flags == 1);
end

if is_train
    filename = 'kitti_train/car_test_greedy.mat';
else
    filename = 'kitti_test/car_test_greedy.mat';
end
save(filename, 'dets_greedy', '-v7.3');

matlabpool close;