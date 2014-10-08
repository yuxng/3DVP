function exemplar_dpm_train_and_test_joint

matlabpool open;

cls = 'car';
is_continue = 0;
is_train = 1;
is_pascal = 1;

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
data.idx = data.idx_ap;

% cluster centers
centers = unique(data.idx);
centers(centers == -1) = [];

exemplar_train_joint(cls, data, centers, '', is_train, is_continue, is_pascal);
exemplar_test_joint(cls, centers, is_train, is_continue, is_pascal);

matlabpool close;