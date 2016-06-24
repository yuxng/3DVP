function cluster_12_classes_pose

classes = {'aeroplane', 'bicycle', 'boat', ...
           'bottle', 'bus', 'car', 'chair', ...
           'diningtable', 'motorbike', 'sofa', 'train', 'tvmonitor'};
num = numel(classes);

K = 24;

% load data
object = load('data.mat');
data = object.data;
N = numel(data.id);

if exist('idxes_pose.mat', 'file')
    object = load('idxes_pose.mat');
    idxes = object.idxes;
else
    idxes = zeros(N, num);
    for i = 1:num
        cls = classes{i};
        idx = cluster_3d_occlusion_patterns(cls, data, 'pose', K);
        idxes(:,i) = idx;
    end
    save('idxes_pose.mat', 'idxes');
end

% combine the idxes
idx = max(idxes, [], 2);
data.idx_pose = idx;
save('data.mat', 'data');