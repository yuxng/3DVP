function train_contextual_svms

result_dir = 'contextual_svms';

% add liblinear path
addpath('../3rd_party/liblinear-1.94/matlab');

is_train = 1;

% load data
if is_train
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_all.mat');
end
data = object.data;
centers = unique(data.idx_ap);
centers(centers == -1) = [];
nc = numel(centers);

% load ids
object = load('kitti_ids.mat');
if is_train
    ids = object.ids_train;
else
    ids = [object.ids_train object.ids_val];
end

% load training data
fprintf('load training data...');
Tdata = [];
for i = 1:numel(ids)
    id = ids(i);
    filename = fullfile('CACHED_DATA_TRAINVAL', sprintf('%04d.mat', id));
    object = load(filename);
    Tdata(i).labels = object.labels;
    Tdata(i).features = object.features;
    Tdata(i).detections = object.detections;
end
fprintf('done\n');

% for each center
for c = 1:nc
    cid = centers(c);
    
    % accumulate labels and features
    labels = [];
    features = [];
    count = 0;
    
    for i = 1:numel(ids)
        if isempty(Tdata(i).detections)
            continue;
        end
        cids = Tdata(i).detections(:,5);
        index = find(cids == cid);
        num = numel(index);
        if num ~= 0
            labels(count+1:count+num) = Tdata(i).labels(index);
            F = full(Tdata(i).features);
            features(count+1:count+num, :) = F(index, :);
            count = count + num;
        end
    end
    
    % train a linear svm
    fprintf('center %d: %d training examples, %d positives, %d negatives\n', cid, count, ...
        numel(find(labels == 1)), numel(find(labels == -1)));
    labels = labels';
    features = sparse(features);
    model = train(labels, features, '-c 1');
    
    % save model
    filename = fullfile(result_dir, sprintf('car_%d_final.mat', cid));
    save(filename, 'model');
end