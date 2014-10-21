function exemplar_write_kitti_train_ids

% read ids of validation images
object = load('kitti_ids_new.mat');
ids = object.ids_train;

num = numel(ids);
fid = fopen('kitti_ids_train.txt', 'w');
fprintf(fid, '%d\n', num);
for i = 1:num
    fprintf(fid, '%d\n', ids(i));
end
fclose(fid);