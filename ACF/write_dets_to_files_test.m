function write_dets_to_files_test

% load ids
object = load('kitti_ids_new.mat');
ids_train = sort([object.ids_train object.ids_val]);
ids_test = object.ids_test;

% load detections
object = load('kitti_test_acf_3d_227_flip/car_3d_ap_227_combined_train.mat');
dets_train = object.dets;
object = load('kitti_test_acf_3d_227_flip/car_3d_ap_227_combined_test.mat');
dets_test = object.dets;

for i = 1:numel(ids_train)
    filename = sprintf('3DVP_227/training/%06d.txt', ids_train(i));
    disp(filename);
    fid = fopen(filename, 'w');

    det = dets_train{i};
    for j = 1:size(det,1)
        fprintf(fid, '%f %f %f %f\n', det(j,1), det(j,2), det(j,3), det(j,4));
    end

    fclose(fid);
end

for i = 1:numel(ids_test)
    filename = sprintf('3DVP_227/testing/%06d.txt', ids_test(i));
    disp(filename);
    fid = fopen(filename, 'w');

    det = dets_test{i};
    for j = 1:size(det,1)
        fprintf(fid, '%f %f %f %f\n', det(j,1), det(j,2), det(j,3), det(j,4));
    end

    fclose(fid);
end