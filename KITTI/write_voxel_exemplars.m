function write_voxel_exemplars

is_train = 0;

% load ids
object = load('kitti_ids_new.mat');
if is_train
    ids = object.ids_train;
    outdir = 'Voxel_exemplars';
else
    ids = sort([object.ids_train object.ids_val]);
    outdir = 'Voxel_exemplars_all';
end

% load data
if is_train
    object_car = load('data.mat');
    object_pedestrian = load('data_pedestrian.mat');
    object_cyclist = load('data_cyclist.mat');
else
    object_car = load('data_kitti.mat');
    object_pedestrian = load('data_kitti_pedestrian.mat');
    object_cyclist = load('data_kitti_cyclist.mat');
end

cls = {'Car', 'Pedestrian', 'Cyclist'};
num_cls = numel(cls);
data = cell(1, num_cls);
data{1} = object_car.data;
data{2} = object_pedestrian.data;
data{3} = object_cyclist.data;
idx = cell(1, num_cls);
idx{1} = data{1}.idx_ap;
idx{2} = data{2}.idx_pose;
idx{3} = data{3}.idx_pose;

% cluster centers
centers = cell(1, num_cls);
for i = 1:num_cls
    centers{i} = unique(idx{i});
    centers{i}(centers{i} == -1) = [];
    N = numel(centers{i});
    fprintf('%d clusters for %s\n', N, cls{i});
end

% write the mapping from cluster center to azimuth and alpha
filename = sprintf('%s/mapping.txt', outdir);
fid = fopen(filename, 'w');
count = 0;
for k = 1:num_cls
    N = numel(centers{k});
    for i = 1:N
        azimuth = data{k}.azimuth(centers{k}(i));
        alpha = azimuth + 90;
        if alpha >= 360
            alpha = alpha - 360;
        end
        alpha = alpha*pi/180;
        if alpha > pi
            alpha = alpha - 2*pi;
        end
        fprintf(fid, '%d %s %f %f\n', i + count, cls{k}, azimuth, alpha);
    end
    count = count + N;
end
fclose(fid);

% for each image
for i = 1:numel(ids)
    id = ids(i);
    filename = sprintf('%s/%06d.txt', outdir, id);
    fid = fopen(filename, 'w');
    
    count = 0;
    count_object = 0;
    for k = 1:num_cls
        % write object info to file
        index = find(data{k}.id == id);
        for j = 1:numel(index)
            ind = index(j);
            % cluster id
            cluster_idx = idx{k}(ind);
            if cluster_idx ~= -1
                cluster_idx = find(centers{k} == cluster_idx) + count;
            end

            % flip
            is_flip = data{k}.is_flip(ind);

            % bounding box
            bbox = data{k}.bbox(:,ind);

            fprintf(fid, '%s %d %d %.2f %.2f %.2f %.2f\n', cls{k}, cluster_idx, is_flip, bbox(1), bbox(2), bbox(3), bbox(4));
        end
        count = count + numel(centers{k});
        count_object = count_object + numel(index);
    end
    
    fclose(fid);
    fprintf('%s: %d objects written\n', filename, count_object);
end