% Set up global variables used throughout the code

% directory for caching models, intermediate data, and results
cachedir = '/scratch/yuxiang/Projects/data/';
resultdir = 'data/';

if exist(cachedir, 'dir') == 0
  unix(['mkdir -p ' cachedir]);
end

if exist(resultdir, 'dir') == 0
  unix(['mkdir -p ' resultdir]);
end

% directory with KITTI development kit and dataset
KITTIpaths = {'/net/acadia/workplace/yuxiang/Projects/KITTI', ...
    '/home/yuxiang/Projects/KITTI_Dataset', ...
    '/scratch/yuxiang/Projects/KITTI_Dataset'};
for i = 1:numel(KITTIpaths)
    if exist(KITTIpaths{i}, 'dir')
        KITTIroot = [KITTIpaths{i} '/data_object_image_2'];
        KITTIdevkit = [KITTIpaths{i} '/devkit/matlab'];
        break;
    end
end
addpath(KITTIdevkit);