function exemplar_dpm_test

% Main function to test exemplar DPM for occlusion patterns

% load occlusion patterns
filename = '../KITTI/data.mat';
object = load(filename);
data = object.data;

% cluster centers
centers = unique(data.idx);

% train an exemplar DPM for each cluster
cls = 'car';
num = numel(centers);
for i = 1:1
    fprintf('%d/%d: Test DPM for center %d\n', i, num, centers(i));
    kitti_test(cls, centers(i));
end