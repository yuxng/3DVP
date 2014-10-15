function idx = cluster_3d_occlusion_patterns(data, algorithm, K)

is_continue = 1;

% select the clustering data
height = data.bbox(4,:) - data.bbox(2,:) + 1;
occlusion = data.occlusion;
truncation = data.truncation;
flag = height > 25 & occlusion < 3 & truncation < 0.5;
fprintf('%d examples in clustering\n', sum(flag));    

switch algorithm
    case 'ap'
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

        p = min(s(:,3));

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
    case 'kmeans'
        fprintf('3d kmeans %d\n', K);
        % try to load distances
        if exist('distances.mat', 'file') ~= 0
            fprintf('load distances from file\n');
            object = load('distances.mat');
            distances = object.distances;
        else
            fprintf('computing distances...\n');
            scores = compute_similarity(data.grid(:,flag));
            distances = 1 - scores;
            for i = 1:size(distances,1)
                distances(i,i) = 0;
            end            

            save('distances.mat', 'distances', '-v7.3');
            fprintf('save distances\n');
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