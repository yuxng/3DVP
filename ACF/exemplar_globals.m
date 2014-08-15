% Set up global variables used throughout the code

% directory for caching models, intermediate data, and results
cachedir = 'data/';

if exist(cachedir, 'dir') == 0
  unix(['mkdir -p ' cachedir]);
end

% directory with KITTI development kit and dataset
KITTIpaths = {'/net/acadia/workplace/yuxiang/Projects/KITTI', ...
    '/home/yuxiang/Projects/KITTI_Dataset', ...
    '/scail/scratch/u/yuxiang/KITTI_Dataset', ...
    '/afs/cs.stanford.edu/group/cvgl/rawdata/KITTI_Dataset'};
for i = 1:numel(KITTIpaths)
    if exist(KITTIpaths{i}, 'dir')
        KITTIroot = [KITTIpaths{i} '/data_object_image_2'];
        KITTIdevkit = [KITTIpaths{i} '/devkit/matlab'];
        break;
    end
end
addpath(KITTIdevkit);