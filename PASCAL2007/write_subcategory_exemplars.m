function write_subcategory_exemplars

% PASCAL path
opt = globals();
pascal_init;

% load ids
ids = textread(sprintf(VOCopts.imgsetpath, 'trainval'), '%s');

outdir = 'subcategory_exemplars';

% load data
object = load('data.mat');
data = object.data;
idx = data.idx;

% cluster centers
centers = unique(idx);
centers(centers == -1) = [];
N = numel(centers);
fprintf('%d clusters\n', N);

% for each image
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
end