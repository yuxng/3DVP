 function [cids, cluster_sizes, width_mean, height_mean] = compute_cluster_statistics

% load data
object = load('data_kitti.mat');
data = object.data;

idx = data.idx_ap;
cids = unique(idx);
cids(cids == -1) = [];
num = numel(cids);

% compute statistics
cluster_sizes = zeros(num, 1);
width_mean = zeros(num, 1);
height_mean = zeros(num, 1);
occ_per = zeros(num, 1);
trunc_per = zeros(num, 1);

for i = 1:num
    cid = cids(i);
    
    cluster_sizes(i) = numel(find(idx == cid));
    
    bbox = data.bbox(:, idx == cid);
    w = bbox(3,:) - bbox(1,:);
    h = bbox(4,:) - bbox(2,:);
    width_mean(i) = mean(w);
    height_mean(i) = mean(h);
    
    occ_per(i) = data.occ_per(cid);
    trunc_per(i) = data.truncation(cid);
end

num_occ = numel(find(occ_per > 0));
num_trunc = numel(find(trunc_per > 0));
num_visible = num - num_occ - num_trunc;

fprintf('%d 3DVPs visible\n', num_visible);
fprintf('%d 3DVPs occluded\n', num_occ);
fprintf('%d 3DVPs truncated\n', num_trunc);