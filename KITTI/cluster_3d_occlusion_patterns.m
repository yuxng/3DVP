function cluster_3d_occlusion_patterns

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

    % compute the similarity matrix
    tmp = double(grid);
    num = sum(tmp, 2);
    scores = tmp*tmp';
    fprintf('finish matrix production\n');
    for i = 1:N
        if num(i) ~= 0
            scores(i,:) = scores(i,:) / num(i);
        end
    end
    
    index = find(scores ~= 0);
    [x, y] = ind2sub(size(scores), index);
    M = numel(x);
    s = zeros(M,3); % Make ALL N^2-N similarities
    for i = 1:M
        s(i,1) = x(i);
        s(i,2) = y(i);
        s(i,3) = scores(x(i), y(i));
    end
    p = median(s(:,3));
    
    save('similarity.mat', 's', 'p');
    fprintf('done\n');
end

% clustering
fprintf('Start AP clustering\n');
[idx, netsim, dpsim, expref] = apclusterSparse(s, p, 'plot');

fprintf('Number of clusters: %d\n', length(unique(idx)));
fprintf('Fitness (net similarity): %f\n', netsim);