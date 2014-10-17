function exemplar_dpm_train_and_test_batch_aps(index)

matlabpool open;

exemplar_globals;
cls = 'car';
is_continue = 0;
is_train = 1;
is_pascal = 0;

if is_pascal
    if is_train
        filename = fullfile(SLMroot, 'PASCAL3D/data.mat');
    else
        filename = fullfile(SLMroot, 'PASCAL3D/data_all.mat');
    end
else
    if is_train
        filename = fullfile(SLMroot, 'KITTI/data.mat');
    else
        filename = fullfile(SLMroot, 'KITTI/data_all.mat');
    end
end

object = load(filename);
data = object.data;
job_indexes = data.job_indexes;

for i = index
    id = job_indexes(i,2);
    ind = job_indexes(i,3);
    if job_indexes(i,1) == 1
        data.idx = data.idx_2d_aps{id};
        name = sprintf('2d_aps_%d', id);
    else
        data.idx = data.idx_3d_aps{id};
        name = sprintf('3d_aps_%d', id);
    end
    
    threshold = -50;
    
    % cluster centers
    centers = unique(data.idx);
    centers(centers == -1) = [];    
    num = numel(centers);    
    
    fprintf('%d/%d: Train DPM for center %d\n', ind, num, centers(ind));
    exemplar_kitti_train(cls, name, data, ind, centers(ind), is_train, is_continue, is_pascal);
    exemplar_kitti_test(cls, name, ind, centers(ind), threshold, is_train, is_continue, is_pascal);
end

matlabpool close;