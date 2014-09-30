function cluster_2d_occlusion_patterns

opt = globals;
data_file = 'data.mat';
is_save = 1;
K = 350;
algorithm = 'ap';

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

switch algorithm
    case 'kmeans'
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
    case 'ap'
        % try to load similarity scores
        if exist('similarity_2d.mat', 'file') ~= 0
            fprintf('load similarity scores from file\n');
            object = load('similarity_2d.mat');
            scores = object.scores;
        else
            fprintf('computing similarity scores...\n');
            scores = compute_similarity_2d(X);
            save('similarity_2d.mat', 'scores', '-v7.3');
            fprintf('save similarity scores\n');
        end
        
        smax = max(max(scores));
        smin = min(min(scores));
        wa = 1 / (smax - smin);
        wb = -smin / (smax - smin);        
        scores = wa .* scores + wb;
        
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

        p = (min(s(:,3)) + median(s(:,3))) * 0.35;

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
        
        % save results
        if is_save
            data.idx_ap = idx;
            save(data_file, 'data');
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