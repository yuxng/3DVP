function cluster_combine_results

data_file = 'data.mat';
object = load(data_file);
data = object.data;

% 2d clusters
ps = data.ps_2d;
num = numel(ps);
idx_2d_aps = cell(1, num);
for i = 1:num
    filename = sprintf('Clusters/idx_2d_ap_%d.mat', i);
    object = load(filename);
    idx_2d_aps{i} = object.idx_2d_ap;
    centers = unique(idx_2d_aps{i});
    centers(centers == -1) = [];
    fprintf('2d ap, %f, %d clusters\n', ps(i), numel(centers));
end

% 3d clusters
ps = data.ps_3d;
num = numel(ps);
idx_3d_aps = cell(1, num);
for i = 1:num
    filename = sprintf('Clusters/idx_3d_ap_%d.mat', i);
    object = load(filename);
    idx_3d_aps{i} = object.idx_3d_ap;
    centers = unique(idx_3d_aps{i});
    centers(centers == -1) = [];    
    fprintf('3d ap, %f, %d clusters\n', ps(i), numel(centers));
end

data.idx_2d_aps = idx_2d_aps;
data.idx_3d_aps = idx_3d_aps;
save(data_file, 'data');