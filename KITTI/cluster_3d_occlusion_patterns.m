function idx = cluster_3d_occlusion_patterns

% try to load similarity scores
if exist('similarity.mat', 'file') ~= 0
    fprintf('load similarity scores from file\n');
    object = load('similarity.mat');
    s = object.s;
    p = object.p;
else
    fprintf('computing similarity scores...\n');
    % load occlusion patterns
    object = load('occlusion_patterns.mat');
    grid = object.grid;
    N = size(grid, 1);

    % compute the intersection and union for voxels
    tmp = double(grid);
    intersection = tmp*tmp';
    fprintf('finish matrix production for intersection\n');
    
    tmp = 1 - tmp;
    union = tmp*tmp';
    fprintf('finish matrix production for union\n');
    
    scores = (intersection + union) / size(grid,2);
    
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
    p = median(s(:,3));
    
    save('similarity.mat', 's', 'p', '-v7.3');
    fprintf('save similarity scores\n');
end

% clustering
fprintf('Start AP clustering\n');
[idx, netsim, dpsim, expref] = apclustermex(s, p);

fprintf('Number of clusters: %d\n', length(unique(idx)));
fprintf('Fitness (net similarity): %f\n', netsim);