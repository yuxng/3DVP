function exemplar_dpm_train_and_test(index)

matlabpool open;

cls = 'car';

% Main function to train exemplar DPM for occlusion patterns

% load the mean CAD model
% filename = sprintf('../Geometry/%s_mean.mat', cls);
% object = load(filename);
% cad = object.(cls);

% load occlusion patterns
is_train = 0;

if is_train
    filename = '../KITTI/data.mat';
else
    filename = '../KITTI/data_all.mat';
end
object = load(filename);
data = object.data;
data.idx = data.idx_kmeans;


% cluster centers
centers = unique(data.idx);
centers(centers == -1) = [];

% train an exemplar DPM for each cluster
num = numel(centers);

if nargin < 1
    index = 1:num;
end

for i = index
    fprintf('%d/%d: Train DPM for center %d\n', i, num, centers(i));
    exemplar_kitti_train(cls, data, centers(i), is_train);
    exemplar_kitti_test(cls, centers(i), is_train);
end

matlabpool close;