function idx = create_idx_ap(data)

% select the clustering data
height = data.bbox(4,:) - data.bbox(2,:) + 1;
occlusion = data.occlusion;
truncation = data.truncation;
flag = height > 25 & occlusion < 3 & truncation < 0.5;
fprintf('%d examples in clustering\n', sum(flag));    

object = load('idx_ap.mat');
idx_ap = object.idx_ap;

fprintf('Number of clusters: %d\n', length(unique(idx_ap)));
% construct idx
num = numel(data.imgname);
idx = zeros(num, 1);
idx(flag == 0) = -1;
index_all = find(flag == 1);

cids = unique(idx_ap);
K = numel(cids);
for i = 1:K
    index = idx_ap == cids(i);
    cid = index_all(cids(i));
    idx(index_all(index)) = cid;
end