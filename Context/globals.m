% directory with KITTI development kit and dataset
KITTIpaths = {'/net/skyserver10/workplace/yxiang/KITTI_Dataset', ...
    '/net/acadia/workplace/yuxiang/Projects/KITTI', ...
    '/home/yuxiang/Projects/KITTI_Dataset', ...
    '/scratch/yuxiang/Projects/KITTI_Dataset'};

for i = 1:numel(KITTIpaths)
    if exist(KITTIpaths{i}, 'dir')
        KITTIroot = [KITTIpaths{i} '/data_object_image_2'];
        KITTIdevkit = [KITTIpaths{i} '/devkit/matlab'];
        break;
    end
end