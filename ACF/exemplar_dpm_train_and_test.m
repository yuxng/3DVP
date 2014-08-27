function exemplar_dpm_train_and_test(index)

if(ischar(index))
    index = str2double(index);
end

exemplar_globals;
if is_hadoop == 0
    matlabpool open;
end

cls = 'car';
is_continue = 0;
is_train = 0;

if is_train
    filename = fullfile(SLMroot, 'KITTI/data.mat');
else
    filename = fullfile(SLMroot, 'KITTI/data_all.mat');
end
object = load(filename);
data = object.data;
data.idx = data.idx_ap;

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
    exemplar_kitti_train(cls, data, centers(i), is_train, is_continue);
    exemplar_kitti_test(cls, centers(i), is_train, is_continue);
end

if is_hadoop == 0
    matlabpool close;
end