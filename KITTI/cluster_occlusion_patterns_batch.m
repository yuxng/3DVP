function cluster_occlusion_patterns_batch(index)

data_file = 'data.mat';
object = load(data_file);
data = object.data;
job_indexes = data.job_indexes;

if job_indexes(index,1) == 1
    is_3d = 0;
else
    is_3d = 1;
end

% algorithm = 'kmeans';
% K = [5, 10, 20, 30, 40, 50, 100, 150, 200, 250, 300, 350];
% num = numel(K);
% idx_2d_kmeans = cell(1, num);
% idx_3d_kmeans = cell(1, num);
% 
% for i = 1:num
%     idx_2d_kmeans{i} = cluster_2d_occlusion_patterns(data, algorithm, K(i));
%     idx_3d_kmeans{i} = cluster_3d_occlusion_patterns(data, algorithm, K(i));
% end
% 
% data.K = K;
% data.idx_2d_kmeans = idx_2d_kmeans;
% data.idx_3d_kmeans = idx_3d_kmeans;
% save(data_file, 'data');

% ps_3d = [0.1, 0.4, 0.7, 1, 1.2, 1.3];
% ps_2d = [2.4, 2.2, 1.9, 1.7, 1.5, 1.3];

algorithm = 'ap';
ind = job_indexes(index,2);
if is_3d
    idx_3d_ap = cluster_3d_occlusion_patterns(data, algorithm, [], data.ps_3d(ind));
    filename = sprintf('Clusters/idx_3d_ap_%d.mat', ind);
    save(filename, 'idx_3d_ap');
else
    idx_2d_ap = cluster_2d_occlusion_patterns(data, algorithm, [], data.ps_2d(ind));
    filename = sprintf('Clusters/idx_2d_ap_%d.mat', ind);
    save(filename, 'idx_2d_ap');
end