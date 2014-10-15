function cluster_occlusion_patterns_batch

data_file = 'data.mat';
object = load(data_file);
data = object.data;

algorithm = 'kmeans';
K = [5, 10, 20, 30, 40, 50, 100, 150, 200, 250, 300, 350];
num = numel(K);
idx_2d_kmeans = cell(1, num);
idx_3d_kmeans = cell(1, num);

for i = 1:num
    idx_2d_kmeans{i} = cluster_2d_occlusion_patterns(data, algorithm, K(i));
    idx_3d_kmeans{i} = cluster_3d_occlusion_patterns(data, algorithm, K(i));
end

data.K = K;
data.idx_2d_kmeans = idx_2d_kmeans;
data.idx_3d_kmeans = idx_3d_kmeans;
save(data_file, 'data');