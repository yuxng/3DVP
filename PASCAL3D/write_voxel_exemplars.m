function write_voxel_exemplars

% PASCAL path
opt = globals();
pascal_init;

% load ids
ids = textread(sprintf(VOCopts.imgsetpath, 'train'), '%s');

outdir = 'voxel_exemplars';

% load data
object = load('data.mat');
data = object.data;
idx = data.idx;

% cluster centers
centers = unique(idx);
centers(centers == -1) = [];
N = numel(centers);
fprintf('%d clusters\n', N);

% write the mapping from cluster center to azimuth and alpha
filename = sprintf('%s/mapping.txt', outdir);
fid = fopen(filename, 'w');

for i = 1:N
    cls = data.cls{centers(i)};
    azimuth = data.azimuth(centers(i));
    fprintf(fid, '%d %s %f\n', i, cls, azimuth);
end

fclose(fid);

% for each image
count = 0;
for i = 1:numel(ids)
    id = ids{i};
    filename = sprintf('%s/%s.txt', outdir, id);
    fid = fopen(filename, 'w');

    % write object info to file
    index = find(strcmp(id, data.id) == 1);
    for j = 1:numel(index)
        ind = index(j);
        % cluster id
        cluster_idx = idx(ind);
        if cluster_idx ~= -1
            cluster_idx = find(centers == cluster_idx);
        end

        % flip
        is_flip = data.is_flip(ind);

        % bounding box
        bbox = data.bbox(:,ind);
        
        % class
        cls = data.cls{ind};

        fprintf(fid, '%s %d %d %.2f %.2f %.2f %.2f\n', cls, cluster_idx, is_flip, bbox(1), bbox(2), bbox(3), bbox(4));
    end
    
    fclose(fid);
    fprintf('%s: %d objects written\n', filename, numel(index));
    count = count + numel(index);
end
fprintf('%d objects written in total\n', count);