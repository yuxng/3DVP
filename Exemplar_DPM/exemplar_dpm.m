function exemplar_dpm

% Main function to train exemplar DPM for occlusion patterns

% load occlusion patterns
filename = '../KITTI/data.mat';
object = load(filename);
data = object.data;

% cluster centers
centers = unique(data.idx);

% train an exemplar DPM for each cluster
cls = 'car';
num = numel(centers);
for i = 1:num
    kitti_train(cls, data, centers(i), '');
end