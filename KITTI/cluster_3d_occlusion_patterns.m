function cluster_3d_occlusion_patterns

data_file = 'data.mat';
is_save = 1;
algorithm = 'kmeans';

switch algorithm
    case 'ap'

        % try to load similarity scores
        if exist('similarity.mat', 'file') ~= 0
            fprintf('load similarity scores from file\n');
            object = load('similarity.mat');
            s = object.s;
        else
            fprintf('computing similarity scores...\n');
            % load data
            object = load(data_file);
            data = object.data;

            scores = compute_similarity(data.grid);

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

            save('similarity.mat', 's', '-v7.3');
            fprintf('save similarity scores\n');
        end

        p = median(s(:,3));

        % clustering
        fprintf('Start AP clustering\n');
        [idx, netsim, dpsim, expref] = apclustermex(s, p);

        fprintf('Number of clusters: %d\n', length(unique(idx)));
        fprintf('Fitness (net similarity): %f\n', netsim);
        
        % save results
        if is_save == 1
            object = load(data_file);
            data = object.data;
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
        flag = height > 40 & occlusion == 0 & truncation < 0.15;        
        
        % load data
        K = 20;
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
    otherwise
        fprintf('algorithm %s not supported\n', algorithm);
end