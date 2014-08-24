function exemplar_dpm_test(index)

matlabpool open;

% Main function to test exemplar DPM for occlusion patterns

% load occlusion patterns
is_train = 1;

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
cls = 'car';
num = numel(centers);

if nargin < 1
    index = 1:num;
end

for i = index
    fprintf('%d/%d: Test DPM for center %d\n', i, num, centers(i));
    exemplar_kitti_test(cls, centers(i), is_train);
end

matlabpool close;