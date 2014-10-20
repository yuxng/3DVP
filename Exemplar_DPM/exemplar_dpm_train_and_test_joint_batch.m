function exemplar_dpm_train_and_test_joint_batch(index)

matlabpool open;

is_continue = 0;
is_train = 1;
is_pascal = 1;

if is_pascal
    if is_train
        filename = '../PASCAL3D/data_all.mat';
    end
else
    if is_train
        filename = '../KITTI/data.mat';
    else
        filename = '../KITTI/data_all.mat';
    end
end

object = load(filename);
data = object.data;
classes = data.classes;
job_indexes = data.job_indexes;
K = data.K;

for i = index
    ind_class = job_indexes(i,2);
    cls = classes{ind_class};
    ind = job_indexes(i,3);
    if job_indexes(i,1) == 1
        data.idx = data.idx_2d_kmeans{ind_class, ind};
        name = sprintf('2d_kmeans_%d', K(ind));
    else
        data.idx = data.idx_3d_kmeans{ind_class, ind};
        name = sprintf('3d_kmeans_%d', K(ind));
    end

    % cluster centers
    centers = unique(data.idx);
    centers(centers == -1) = [];
    fprintf('%d clusters\n', numel(centers));

    exemplar_train_joint(cls, name, data, centers, '', is_train, is_continue, is_pascal);
    exemplar_test_joint(cls, name, is_train, is_continue, is_pascal);
end

matlabpool close;