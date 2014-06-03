function opt = globals()

path_pascal3d = '/home/ma/yxiang/Projects/PASCAL3D+_release1.1';
path_kitti = '/home/ma/yxiang/Projects/KITTI';
 
opt.path_pascal3d = path_pascal3d;
opt.path_img_pascal = [opt.path_pascal3d '/Images/%s_pascal'];
opt.path_ann_pascal = [opt.path_pascal3d '/Annotations/%s_pascal'];
opt.path_img_imagenet = [opt.path_pascal3d '/Images/%s_imagenet'];
opt.path_ann_imagenet = [opt.path_pascal3d '/Annotations/%s_imagenet'];
opt.path_pascal = [opt.path_pascal3d '/PASCAL/VOCdevkit'];
opt.path_cad = [opt.path_pascal3d '/CAD/%s.mat'];

opt.path_kitti = path_kitti;
opt.path_kitti_devkit = [opt.path_kitti '/devkit/matlab'];
opt.path_kitti_root = [opt.path_kitti '/data_object_image_2'];

% add kitti devit path
addpath(opt.path_kitti_devkit);