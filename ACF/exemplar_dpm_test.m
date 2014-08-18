function exemplar_dpm_test(index)

matlabpool open;

% Main function to test exemplar DPM for occlusion patterns

% load occlusion patterns
filename = '../KITTI/data_all.mat';
object = load(filename);
data = object.data;
data.idx = data.idx_ap;
is_train = 0;

% cluster centers
centers = unique(data.idx);

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