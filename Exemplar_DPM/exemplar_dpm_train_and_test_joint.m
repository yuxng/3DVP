function exemplar_dpm_train_and_test_joint

matlabpool open;

cls = 'car';
is_continue = 0;
is_train = 1;
is_pascal = 0;

if is_pascal
    if is_train
        filename = '../PASCAL3D/data.mat';
    else
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
data.idx = data.idx_pose;

% cluster centers
centers = unique(data.idx);
centers(centers == -1) = [];
fprintf('%d clusters\n', numel(centers));
name = sprintf('pose_%d', numel(centers));

exemplar_train_joint(cls, name, data, centers, '', is_train, is_continue, is_pascal);
exemplar_test_joint(cls, name, is_train, is_continue, is_pascal);

matlabpool close;