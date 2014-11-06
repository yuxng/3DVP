is_hadoop = 0;
is_train = 1;

if is_hadoop
    rootpath = '/net/skyserver30/workplace/local/wongun/yuxiang/SLM/';
    image_dir = '/net/skyserver10/workplace/yxiang/KITTI_Dataset/data_object_image_2/training/image_2';
    datapath = fullfile(rootpath, 'KITTI');
    datafile = fullfile(rootpath, 'KITTI/data.mat');
    detfile = fullfile(rootpath, 'ACF/data/car_3d_ap_125_combined_test.mat');
    outpath = fullfile(rootpath, 'ContextW/data/');
else
    rootpath = '/home/yuxiang/Projects/SLM/';
    image_dir = '/home/yuxiang/Projects/KITTI_Dataset/data_object_image_2/training/image_2';
    datapath = fullfile(rootpath, 'KITTI');
    if is_train
        datafile = fullfile(rootpath, 'KITTI/data.mat');
        detfile = fullfile(rootpath, 'ACF/kitti_train_ap_125/car_3d_aps_125_combined_test.mat');
        outpath = fullfile(rootpath, 'ContextW/kitti_train_ap_125/');
    else
        datafile = fullfile(rootpath, 'KITTI/data_kitti.mat');
        detfile = fullfile(rootpath, 'ACF/kitti_test_acf_3d_227_flip/car_3d_ap_227_combined_test.mat');
        outpath = fullfile(rootpath, 'ContextW/kitti_test_ap_227/');
    end        
end