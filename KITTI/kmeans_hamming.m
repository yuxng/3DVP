function idx = kmeans_hamming(distances, K, opts)

% initialization
N = size(distances, 1);
perm = randperm(N);
centers = perm(1:K);

sse0 = inf;
iter = 0;
while iter < opts.maxiters

    tic;    
    [centers, sse] = kmiter(distances, centers);    
    t = toc;

    if opts.verbose
        fprintf('iter %d: sse = %g (%g secs)\n', iter, sse, t)
    end
    
    if sse0 - sse < opts.mindelta
        break
    end

    sse0 = sse;
    iter = iter+1;
end

% assign points to centers
idx = zeros(N, 1);
for i = 1:N
    d = distances(i, centers);
    [~, ind] = min(d);
    idx(i) = centers(ind);
end


function [C, sse] = kmiter(distances, centers)

N = size(distances, 1);
K = numel(centers);
C = zeros(size(centers));
sse = 0;

% assign points to centers
assignment = zeros(1,N);
for i = 1:N
    d = distances(i, centers);
    [dmin, ind] = min(d);
    sse = sse + dmin;
    assignment(i) = ind;
end

% compute new centers
for k = 1:K
    index = find(assignment == k);
    d = sum(distances(index, index), 2);
    [~, ind] = min(d);
    C(k) = index(ind);
end