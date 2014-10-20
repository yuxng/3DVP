function data = cluster_occlusion_patterns_batch

% load data
data_file = 'data_all.mat';
object = load(data_file);
data = object.data;
classes = data.classes;
N = numel(classes);

algorithm = 'kmeans';
K = [5, 10, 15, 20, 25, 30];
num = numel(K);
idx_2d_kmeans = cell(N, num);
idx_3d_kmeans = cell(N, num);

for i = 1:N
    cls = classes{i};
    for j = 1:num
        idx_2d_kmeans{i, j} = cluster_2d_occlusion_patterns(cls, data, algorithm, K(j), []);
        idx_3d_kmeans{i, j} = cluster_3d_occlusion_patterns(cls, data, algorithm, K(j), []);
    end
end

data.K = K;
data.idx_2d_kmeans = idx_2d_kmeans;
data.idx_3d_kmeans = idx_3d_kmeans;