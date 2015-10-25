function idx = cluster_3d_occlusion_patterns(data, algorithm, K, pscale)

is_continue = 1;

% select the clustering data
height = data.bbox(4,:) - data.bbox(2,:) + 1;
occlusion = data.occlusion;
truncation = data.truncation;
flag = height > 25 & occlusion < 3 & truncation < 0.5;
fprintf('%d examples in clustering\n', sum(flag));    

switch algorithm
    case 'ap'
        fprintf('3d AP %f\n', pscale);
        % try to load similarity scores
        if is_continue == 1 && exist('similarity.mat', 'file') ~= 0
            fprintf('load similarity scores from file\n');
            object = load('similarity.mat');
            scores = object.scores;
        else
            fprintf('computing similarity scores...\n');
            scores = compute_similarity(data.grid(:,flag));
            save('similarity.mat', 'scores', '-v7.3');
            fprintf('save similarity scores\n');
        end
        
        for i = 1:size(scores,1)
            scores(i,i) = 1;
        end

        p = min(min(scores)) * pscale;
        disp(p);

        % clustering
        fprintf('Start AP clustering\n');
        [idx_ap, netsim, dpsim, expref] = apclustermex(scores, p);

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
    case 'kmeans'
        fprintf('3d kmeans %d\n', K);
        % try to load distances
        if is_continue == 1 && exist('similarity.mat', 'file') ~= 0
            fprintf('load similarity scores from file\n');
            object = load('similarity.mat');
            scores = object.scores;
        else
            fprintf('computing similarity scores...\n');
            scores = compute_similarity(data.grid(:,flag));
            save('similarity.mat', 'scores', '-v7.3');
            fprintf('save similarity scores\n');
        end
        
        distances = 1 - scores;
        for i = 1:size(distances,1)
            distances(i,i) = 0;
        end            
        
        % load data
        opts = struct('maxiters', 1000, 'mindelta', eps, 'verbose', 1);
        idx_kmeans = kmeans_hamming(distances, K, opts);
        
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
    case 'pose'        
        % split the azimuth
        vnum = K;
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
            if isempty(index) == 0
                ind = 1;
                cid = index_all(index(ind));
                idx(index_all(index)) = cid;
            end
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