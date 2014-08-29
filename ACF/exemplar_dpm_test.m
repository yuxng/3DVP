function exemplar_dpm_test(index)

matlabpool open;

% Main function to test exemplar DPM for occlusion patterns

% load occlusion patterns
is_continue = 0;
is_train = 0;

if is_train
    filename = '../KITTI/data.mat';
else
    filename = '../KITTI/data_all.mat';
end
object = load(filename);
data = object.data;
data.idx = data.idx_ap;


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
    exemplar_kitti_test(cls, i, centers(i), is_train, is_continue);
end

matlabpool close;