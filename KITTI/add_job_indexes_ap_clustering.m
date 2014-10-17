function add_job_indexes_ap_clustering

data_file = 'data.mat';
object = load(data_file);
data = object.data;

ps_2d = data.ps_2d;
ps_3d = data.ps_3d;

job_indexes = [];
count = 0;
for k = 1:2
    if k == 1
        ps = ps_2d;
    else
        ps = ps_3d;
    end
    for i = 1:numel(ps)
        count = count + 1;
        job_indexes(count,:) = [k i];
    end
end
data.job_indexes = job_indexes;

save(data_file, 'data');