function data = cluster_occlusion_patterns_batch_bottle

% load data
data_file = 'data_all.mat';
object = load(data_file);
data = object.data;
classes = data.classes;

algorithm = 'kmeans';
K = [5, 10, 15, 20, 25, 30];
num = numel(K);

for i = 4
    cls = classes{i};
    for j = 1:num
        data.idx_2d_kmeans{i, j} = cluster_2d_occlusion_patterns(cls, data, algorithm, K(j), []);
    end
end