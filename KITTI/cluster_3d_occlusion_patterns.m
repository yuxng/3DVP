function cluster_3d_occlusion_patterns

data_file = 'data_all.mat';
is_save = 1;
algorithm = 'ap';

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
        % try to load distances
        if exist('distances.mat', 'file') ~= 0
            fprintf('load distances from file\n');
            object = load('distances.mat');
            distances = object.distances;
        else
            fprintf('computing distances...\n');
            % load data
            object = load(data_file);
            data = object.data;

            scores = compute_similarity(data.grid);
            distances = 1 - scores;

            save('distances.mat', 'distances', '-v7.3');
            fprintf('save distances\n');
        end        
        
        % load data
        K = 30;
        opts = struct('maxiters', 1000, 'mindelta', eps, 'verbose', 1);
        idx = kmeans_hamming(distances, K, opts);
        
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