function add_job_indexes_aps

data_file = 'data.mat';
object = load(data_file);
data = object.data;

job_indexes = [];
count = 0;
for k = 1:2
    if k == 1
        idx_aps = data.idx_2d_aps;
    else
        idx_aps = data.idx_3d_aps;
    end
    for i = 1:numel(idx_aps)
        idx = idx_aps{i};
        cids = unique(idx);
        cids(cids == -1) = [];
        num = numel(cids);
        for j = 1:num
            count = count + 1;
            job_indexes(count,:) = [k i j];
        end
    end
end
data.job_indexes = job_indexes;

save(data_file, 'data');