function save_training_parameters

rootpath = '/home/yuxiang/Projects/SLM/';
datapath = fullfile(rootpath, 'KITTI');
detfile = fullfile(rootpath, 'ACF/kitti_train_ap_125/car_3d_aps_125_combined_test.mat');

load(fullfile(datapath, 'data.mat'));
load(detfile);

params = learn_params(data, dets, {});

save('training_params.mat', 'params');