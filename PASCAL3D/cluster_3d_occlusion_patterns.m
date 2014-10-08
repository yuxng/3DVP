function cluster_3d_occlusion_patterns

data_file = 'data.mat';
is_save = 1;
cls = 'car';
algorithm = 'ap';

switch algorithm
    case 'ap'
        % load data
        object = load(data_file);
        data = object.data;
        
        % select the clustering data        
        cls_ind = find(strcmp(cls, data.classes) == 1);
        height = data.bbox(4,:) - data.bbox(2,:) + 1;
        occlusion = data.occ_per;
        truncation = data.trunc_per;
        flag = data.cls_ind == cls_ind & data.difficult == 0 & ...
            height > 25 & occlusion < 0.5 & truncation < 0.5;
%         flag = data.cls_ind == cls_ind & data.difficult == 0;
        fprintf('%d %s examples in clustering\n', sum(flag), cls);
        
        % collect patterns
        index = find(flag == 1);
        num = numel(index);
        X = [];
        for i = 1:num
            X(:,i) = data.grid{index(i)};
        end

        fprintf('computing similarity scores...\n');
        scores = compute_similarity(X);
        
        N = size(scores, 1);
        M = N*N-N;
        s = zeros(M,3); % Make ALL N^2-N similarities
        j = 1;
        for i = 1:N
            for k = [1:i-1,i+1:N]
                s(j,1) = i;
                s(j,2) = k;
                s(j,3) = scores(i,k);
                j = j+1;
            end
        end       

        p = min(s(:,3)) * 0.1;

        % clustering
        fprintf('Start AP clustering\n');
        [idx_ap, netsim, dpsim, expref] = apclustermex(s, p);

        fprintf('Number of clusters: %d\n', length(unique(idx_ap)));
        fprintf('Fitness (net similarity): %f\n', netsim);
        
        % construct idx
        num = numel(data.imgname);
        idx = zeros(num, 1);
        idx(flag == 0) = -1;
        index_all = find(flag == 1);
        
        cids = unique(idx_ap);
        K = numel(cids);
        for i = 1:K
            index = idx_ap == cids(i);
            cid = index_all(cids(i));
            idx(index_all(index)) = cid;
        end        
        
        % save results
        if is_save == 1
            data.idx_ap = idx;
            save(data_file, 'data');
        end        
        
    case 'kmeans'
        % load data
        object = load(data_file);
        data = object.data;
        
        % try to load distances
        if exist('distances.mat', 'file') ~= 0
            fprintf('load distances from file\n');
            object = load('distances.mat');
            distances = object.distances;
            for i = 1:size(distances,1)
                distances(i,i) = 0;
            end
        else
            fprintf('computing distances...\n');
            scores = compute_similarity(data.grid);
            distances = 1 - scores;

            save('distances.mat', 'distances', '-v7.3');
            fprintf('save distances\n');
        end
        
        % select the clustering data
        height = data.bbox(4,:) - data.bbox(2,:) + 1;
        occlusion = data.occlusion;
        truncation = data.truncation;
        flag = height > 25 & occlusion < 3 & truncation < 0.5;
        fprintf('%d examples in clustering\n', sum(flag));
        
        % load data
        K = 350;
        opts = struct('maxiters', 1000, 'mindelta', eps, 'verbose', 1);
        idx_kmeans = kmeans_hamming(distances(flag, flag), K, opts);
        
        % construct idx
        num = numel(data.imgname);
        idx = zeros(num, 1);
        idx(flag == 0) = -1;
        index_all = find(flag == 1);
        
        cids = unique(idx_kmeans);
        for i = 1:K
            index = idx_kmeans == cids(i);
            cid = index_all(cids(i));
            idx(index_all(index)) = cid;
        end        
        
        % save results
        if is_save == 1
            object = load(data_file);
            data = object.data;
            data.idx_kmeans = idx;
            save(data_file, 'data');
        end
    case 'pose'
        % load data
        object = load(data_file);
        data = object.data;
        
        % select the clustering data
        height = data.bbox(4,:) - data.bbox(2,:) + 1;
        occlusion = data.occlusion;
        truncation = data.truncation;
        flag = height > 40 & occlusion == 0 & truncation < 0.15;
        fprintf('%d examples in clustering\n', sum(flag));
        
        % split the azimuth
        vnum = 16;
        azimuth = data.azimuth(flag);
        num = numel(azimuth);
        idx_pose = zeros(num, 1);
        for i = 1:num
            idx_pose(i) = find_interval(azimuth(i), vnum);
        end
        
        % construct idx
        num = numel(data.imgname);
        idx = zeros(num, 1);
        idx(flag == 0) = -1;
        index_all = find(flag == 1);
        for i = 1:vnum
            index = find(idx_pose == i);
            ind = 1;
            cid = index_all(index(ind));
            idx(index_all(index)) = cid;
        end
        
        % save results
        if is_save == 1
            object = load(data_file);
            data = object.data;
            data.idx_pose = idx;
            save(data_file, 'data');
        end        
        
    otherwise
        fprintf('algorithm %s not supported\n', algorithm);
end

function ind = find_interval(azimuth, num)

a = (360/(num*2)):(360/num):360-(360/(num*2));

for i = 1:numel(a)
    if azimuth < a(i)
        break;
    end
end
ind = i;
if azimuth > a(end)
    ind = 1;
end