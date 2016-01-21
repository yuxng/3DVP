function opt = globals()

path_pascal3d = '/net/acadia/workplace/yuxiang/Projects/PASCAL3D+_release1.1';
if exist(path_pascal3d, 'dir') == 0
    path_pascal3d = '/home/yuxiang/Projects/PASCAL3D+_release1.1';
end
if exist(path_pascal3d, 'dir') == 0
    path_pascal3d = '/scail/scratch/u/yuxiang/PASCAL3D+_release1.1';
end

path_kitti = '/net/acadia/workplace/yuxiang/Projects/KITTI';
if exist(path_kitti, 'dir') == 0
    path_kitti = '/home/yuxiang/Projects/KITTI_Dataset';
end
if exist(path_kitti, 'dir') == 0
    path_kitti = '/scail/scratch/u/yuxiang/KITTI_Dataset';
end

path_slm = '/net/acadia/workplace/yuxiang/Projects/SLM';
if exist(path_slm, 'dir') == 0
    path_slm = '/home/yuxiang/Projects/SLM';
end
if exist(path_slm, 'dir') == 0
    path_slm = '/scail/scratch/u/yuxiang/SLM';
end
 
opt.path_pascal3d = path_pascal3d;
opt.path_img_pascal = [opt.path_pascal3d '/Images/%s_pascal'];
opt.path_ann_pascal = [opt.path_pascal3d '/Annotations/%s_pascal'];
opt.path_img_imagenet = [opt.path_pascal3d '/Images/%s_imagenet'];
opt.path_ann_imagenet = [opt.path_pascal3d '/Annotations/%s_imagenet'];
opt.path_set_imagenet_train = [opt.path_pascal3d '/Image_sets/%s_imagenet_train.txt'];
opt.path_set_imagenet_val = [opt.path_pascal3d '/Image_sets/%s_imagenet_val.txt'];
opt.path_pascal = [opt.path_pascal3d '/PASCAL/VOCdevkit'];
opt.path_cad = [opt.path_pascal3d '/CAD/%s.mat'];

opt.path_kitti = path_kitti;
opt.path_kitti_devkit = [opt.path_kitti '/devkit/matlab'];
opt.path_kitti_root = [opt.path_kitti '/data_object_image_2'];

opt.path_slm = path_slm;
opt.path_slm_geometry = [opt.path_slm '/Geometry'];
opt.path_slm_acf = [opt.path_slm '/ACF'];
opt.path_kmeans = [opt.path_slm '/3rd_party/vggkmeans'];

% add kitti devit path
addpath(opt.path_kitti_devkit);
addpath(opt.path_slm_geometry);
addpath(opt.path_slm_acf);
addpath(opt.path_kmeans);