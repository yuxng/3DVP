function cluster_20_classes

classes = {'aeroplane', 'bicycle', 'bird', 'boat', ...
           'bottle', 'bus', 'car', 'cat', 'chair', ...
           'cow', 'diningtable', 'dog', 'horse', ...
           'motorbike', 'person', 'pottedplant', ...
           'sheep', 'sofa', 'train', 'tvmonitor'};
num = numel(classes);
K = 12;

% load data
object = load('data.mat');
data = object.data;
N = numel(data.id);

if exist('idxes.mat', 'file')
    object = load('idxes.mat');
    idxes = object.idxes;
else
    idxes = zeros(N, num);
    for i = 1:num
        cls = classes{i};
        idx = cluster_2d_occlusion_patterns(cls, data, K);
        idxes(:,i) = idx;
    end
    save('idxes.mat', 'idxes');
end

% combine the idxes
idx = max(idxes, [], 2);
data.idx = idx;
save('data.mat', 'data');