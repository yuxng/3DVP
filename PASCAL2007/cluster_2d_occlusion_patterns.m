function idx = cluster_2d_occlusion_patterns(cls, data, K)

opt = globals;
pascal_init;

% select the clustering data
flag = strcmp(cls, data.cls) == 1 & data.difficult == 0;
fprintf('%d %s examples in clustering\n', sum(flag), cls);

% determine the canonical size of the bounding boxes
modelDs = compute_model_size(data.bbox(:,flag));

% compute features
if strcmp(cls, 'bottle') == 1
    sbin = 4;
else
    sbin = 8;
end
index = find(flag == 1);
X = [];
fprintf('computing features...\n');
for i = 1:numel(index)
    ind = index(i);
    % read the image
    id = data.id{ind};
    filename = sprintf(VOCopts.imgpath, id);
    I = imread(filename);
    if data.is_flip(ind) == 1
        I = I(:, end:-1:1, :);
    end
    % crop image
    bbox = data.bbox(:,ind);
    gt = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
    Is = bbApply('crop', I, gt, 'replicate', modelDs([2 1]));
    C = features(double(Is{1}), sbin);
    X(:,i) = C(:);
end
fprintf('done\n');

% AP clustering
fprintf('computing similarity scores...\n');
similarity = compute_similarity_2d(X);
fprintf('done\n');

% clustering
fprintf('Start AP clustering\n');
idx_ap = apclusterK(similarity, K);
fprintf('Number of clusters: %d\n', length(unique(idx_ap)));

% construct idx
num = numel(data.imgname);
idx = zeros(num, 1);
idx(flag == 0) = -1;
index_all = find(flag == 1);

cids = unique(idx_ap);
assert(K == numel(cids));
for i = 1:K
    index = idx_ap == cids(i);
    cid = index_all(cids(i));
    idx(index_all(index)) = cid;
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