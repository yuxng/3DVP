function exemplar_dpm_train(index)

matlabpool open;

cls = 'car';

% Main function to train exemplar DPM for occlusion patterns

% load the mean CAD model
% filename = sprintf('../Geometry/%s_mean.mat', cls);
% object = load(filename);
% cad = object.(cls);

% load occlusion patterns
filename = '../KITTI/data.mat';
object = load(filename);
data = object.data;

% cluster centers
centers = unique(data.idx_ap);

% train an exemplar DPM for each cluster
num = numel(centers);

if nargin < 1
    index = 1:num;
end

for i = index
    fprintf('%d/%d: Train DPM for center %d\n', i, num, centers(i));
    exemplar_kitti_train(cls, data, centers(i));
end

matlabpool close;