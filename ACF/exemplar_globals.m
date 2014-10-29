% Set up global variables used throughout the code

is_hadoop = 1;

% directory for caching models, intermediate data, and results
if is_hadoop
    rootdir = '/net/skyserver30/workplace/local/wongun/yuxiang/SLM/ACF/';
    cachedir = fullfile(rootdir, 'cache/');
    resultdir = fullfile(rootdir, 'data/');
else
    rootdir = '/tmp/yuxiang/Projects';
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
    '/scratch/yuxiang/Projects/KITTI_Dataset', ...
    '/scail/scratch/u/yuxiang/KITTI_Dataset'};

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

SLMpaths = {'/net/skyserver30/workplace/local/wongun/yuxiang/SLM', ...
    '/net/skyserver10/workplace/yxiang/SLM', ...
    '/home/yuxiang/Projects/SLM', ...
    '/scail/scratch/u/yuxiang/SLM'};

for i = 1:numel(SLMpaths)
    if exist(SLMpaths{i}, 'dir')
        SLMroot = SLMpaths{i};
        break;
    end
end

return;

PASCAL3Dpaths = {'/home/yuxiang/Projects/Pose_Dataset/PASCAL3D+_release1.1', ...
    '/scratch/yuxiang/Projects/PASCAL3D+_release1.1', ...
    '/scail/scratch/u/yuxiang/PASCAL3D+_release1.1'};

for i = 1:numel(PASCAL3Dpaths)
    if exist(PASCAL3Dpaths{i}, 'dir')
        PASCAL3Droot = PASCAL3Dpaths{i};
        path_pascal = [PASCAL3Droot '/PASCAL/VOCdevkit'];
        path_img_imagenet = [PASCAL3Droot '/Images/%s_imagenet'];
        break;
    end
end

% pascal init
tmp = pwd;
cd(path_pascal);
addpath([cd '/VOCcode']);
VOCinit;
cd(tmp);