function exemplar_transform_bbox

cls = 'car';
result_dir = 'kitti_train_ap_125';
name = '3d_aps_125_combined';

% read detection results
filename = sprintf('%s/%s_%s_train.mat', result_dir, cls, name);
object = load(filename);
dets = object.dets;
fprintf('load detection done\n');

for i = 1:numel(dets)
    det = dets{i};
    dets{i} = [det(:,1) det(:,2) det(:,1)+det(:,3) det(:,2)+det(:,4) det(:,5) det(:,6)];
end

save(filename, 'dets', '-v7.3');