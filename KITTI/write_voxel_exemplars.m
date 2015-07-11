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
    object = load('data.mat');
else
    object = load('data_kitti.mat');
end
data = object.data;
idx = data.idx_ap;

% cluster centers
centers = unique(idx);
centers(centers == -1) = [];
N = numel(centers);
fprintf('%d clusters\n', N);

% write the mapping from cluster center to azimuth and alpha
filename = sprintf('%s/mapping.txt', outdir);
fid = fopen(filename, 'w');
for i = 1:N
    azimuth = data.azimuth(centers(i));
    alpha = azimuth + 90;
    if alpha >= 360
        alpha = alpha - 360;
    end
    alpha = alpha*pi/180;
    if alpha > pi
        alpha = alpha - 2*pi;
    end
    fprintf(fid, '%d %f %f\n', i, azimuth, alpha);
end
fclose(fid);

% for each image
for i = 1:numel(ids)
    id = ids(i);
    filename = sprintf('%s/%06d.txt', outdir, id);
    fid = fopen(filename, 'w');
    
    % write object info to file
    index = find(data.id == id);
    for j = 1:numel(index)
        ind = index(j);
        % cluste id
        cluster_idx = idx(ind);
        if cluster_idx ~= -1
            cluster_idx = find(centers == cluster_idx);
        end
        
        % flip
        is_flip = data.is_flip(ind);
        
        % bounding box
        bbox = data.bbox(:,ind);
        
        fprintf(fid, '%d %d %.2f %.2f %.2f %.2f\n', cluster_idx, is_flip, bbox(1), bbox(2), bbox(3), bbox(4));
    end
    
    fclose(fid);
    fprintf('%s: %d objects written\n', filename, numel(index));
end