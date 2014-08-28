% Set up global variables used throughout the code

is_hadoop = 1;

% directory for caching models, intermediate data, and results
if is_hadoop
    rootdir = '/workplace/hadoop_cache/slm/';
    cachedir = fullfile(rootdir, 'cache/');
    resultdir = '/net/skyserver10/workplace/yxiang/SLM/ACF/data/';
else
    rootdir = '/scratch/yuxiang/Projects';
    cachedir = fullfile(rootdir, 'data/');
    resultdir = 'data/';
end

if exist(rootdir, 'dir') == 0
    mkdir(rootdir);
end

if exist(cachedir, 'dir') == 0
  unix(['mkdir -p ' cachedir]);
end

if exist(resultdir, 'dir') == 0
  unix(['mkdir -p ' resultdir]);
end

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

if is_hadoop == 0
    addpath(KITTIdevkit);
end

SLMpaths = {'/net/skyserver10/workplace/yxiang/SLM', ...
    '/home/yuxiang/Projects/SLM', ...
    '/scail/scratch/u/yuxiang/SLM'};

for i = 1:numel(SLMpaths)
    if exist(SLMpaths{i}, 'dir')
        SLMroot = SLMpaths{i};
        break;
    end
end
