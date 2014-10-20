function idx = cluster_2d_occlusion_patterns(cls, data, algorithm, K, pscale)

opt = globals;
pascal_init;

% select the clustering data        
cls_ind = find(strcmp(cls, data.classes) == 1);
flag = data.cls_ind == cls_ind & data.difficult == 0 & data.is_pascal == 1;
fprintf('%d %s examples in clustering\n', sum(flag), cls);

% determine the canonical size of the bounding boxes
modelDs = compute_model_size(data.bbox(:,flag));

% compute features
index = find(flag == 1);
X = [];
fprintf('computing features...\n');
for i = 1:numel(index)
    ind = index(i);
    % read the image
    id = data.id{ind};
    if data.is_pascal(ind) == 1
        filename = sprintf(VOCopts.imgpath, id);
    else
        filename = [sprintf(path_img_imagenet, cls) '/' id '.JPEG'];
    end           
    I = imread(filename);
    if data.is_flip(ind) == 1
        I = I(:, end:-1:1, :);
    end           
    % crop image
    bbox = data.bbox(:,ind);
    gt = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
    Is = bbApply('crop', I, gt, 'replicate', modelDs([2 1]));
    C = features(double(Is{1}), 8);
    X(:,i) = C(:);
end
fprintf('done\n');


switch algorithm
    case 'kmeans'
        % kmeans clustering
        fprintf('%s 2d kmeans %d\n', cls, K);
        opts = struct('maxiters', 1000, 'mindelta', eps, 'verbose', 1);
        [center, sse] = vgg_kmeans(X, K, opts);
        [idx_kmeans, d] = vgg_nearest_neighbour(X, center);

        % construct idx
        num = numel(data.imgname);
        idx = zeros(num, 1);
        idx(flag == 0) = -1;
        index_all = find(flag == 1);
        for i = 1:K
            index = find(idx_kmeans == i);
            [~, ind] = min(d(index));
            cid = index_all(index(ind));
            idx(index_all(index)) = cid;
        end
    case 'ap'
        fprintf('2d AP %f\n', pscale);
        fprintf('computing similarity scores...\n');
        scores = compute_similarity_2d(X);
        fprintf('done\n');
        
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

        p = min(s(:,3)) * pscale;

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
end

function modelDs = compute_model_size(bbox)

% pick mode of aspect ratios
h = bbox(4,:) - bbox(2,:) + 1;
w = bbox(3,:) - bbox(1,:) + 1;
xx = -2:.02:2;
filter = exp(-[-100:100].^2/400);
aspects = hist(log(h./w), xx);
aspects = convn(aspects, filter, 'same');
[~, I] = max(aspects);
aspect = exp(xx(I));

% pick 20 percentile area
areas = sort(h.*w);
area = areas(max(floor(length(areas) * 0.2), 1));
area = max(min(area, 10000), 500);

% pick dimensions
w = sqrt(area/aspect);
h = w*aspect;
modelDs = round([h w]);