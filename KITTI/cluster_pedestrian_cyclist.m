function cluster_pedestrian_cyclist

K = 24;

filename = 'data_pedestrian.mat';
disp(filename);
object = load(filename);
data = object.data;
idx = cluster_3d_occlusion_patterns(data, 'pose', K);
data.idx_pose = idx;
save(filename, 'data');

filename = 'data_cyclist.mat';
disp(filename);
object = load(filename);
data = object.data;
idx = cluster_3d_occlusion_patterns(data, 'pose', K);
data.idx_pose = idx;
save(filename, 'data');

K = 36;

filename = 'data_kitti_pedestrian.mat';
disp(filename);
object = load(filename);
data = object.data;
idx = cluster_3d_occlusion_patterns(data, 'pose', K);
data.idx_pose = idx;
save(filename, 'data');

filename = 'data_kitti_cyclist.mat';
disp(filename);
object = load(filename);
data = object.data;
idx = cluster_3d_occlusion_patterns(data, 'pose', K);
data.idx_pose = idx;
save(filename, 'data');