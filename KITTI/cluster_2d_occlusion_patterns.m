function cluster_2d_occlusion_patterns

opt = globals;
data_file = 'data.mat';
is_save = 1;
K = 250;

% get image directory
root_dir = opt.path_kitti_root;
data_set = 'training';
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% load data
object = load(data_file);
data = object.data;

% select the clustering data
height = data.bbox(4,:) - data.bbox(2,:) + 1;
occlusion = data.occlusion;
truncation = data.truncation;
flag = height > 25 & occlusion < 3 & truncation < 0.5;

% determine the canonical size of the bounding boxes
modelDs = compute_model_size(data.bbox(:,flag));

% compute features
if exist('feature.mat', 'file') ~= 0
    fprintf('load features from file\n');
    object = load('feature.mat');
    X = object.X;
else
    pChns = chnsCompute();
    index = find(flag == 1);
    X = [];
    fprintf('computing features...\n');
    for i = 1:numel(index)
        ind = index(i);
        % read the image
        filename = fullfile(image_dir, data.imgname{ind});
        I = imread(filename);
        % crop image
        bbox = data.bbox(:,ind);
        gt = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
        Is = bbApply('crop', I, gt, 'replicate', modelDs([2 1]));
        C = chnsCompute(Is{1}, pChns);
        C = cat(3, C.data{:});
        X(:,i) = C(:);
    end
    fprintf('done\n');
    save('feature.mat', 'X');
end

% kmeans clustering
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

if is_save
    data.idx_kmeans = idx;
    save(data_file, 'data');
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