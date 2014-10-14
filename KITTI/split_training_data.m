function split_training_data

% KITTI path
opt = globals();
dir_mapping = [opt.path_kitti '/devkit/mapping'];
root_dir = opt.path_kitti_root;
data_set = 'training';
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% read mapping file
[dates, seqs, ids] = textread(fullfile(dir_mapping, 'train_mapping.txt'), '%s %s %s');
train_rand = textread(fullfile(dir_mapping, 'train_rand.txt'), '%d', 'delimiter', ',');

[~, index] = sort(train_rand);

ids_train = sort(index(1:3682) - 1)';
ids_val = sort(index(3683:end) - 1)';
ids_test = 0:7517;

save('kitti_ids_new.mat', 'ids_train', 'ids_val', 'ids_test');

% write training and validataion images
% for i = 1:numel(ids_val)
%     img_idx = ids_val(i);
%     disp(img_idx);
%     filename_src = sprintf('%s/%06d.png', image_dir, img_idx);
%     filename_dst = sprintf('val/%06d.png', i - 1);
%     copyfile(filename_src, filename_dst);
% end