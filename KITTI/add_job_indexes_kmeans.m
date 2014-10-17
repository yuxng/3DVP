function add_job_indexes_kmeans

data_file = 'data.mat';
object = load(data_file);
data = object.data;
K = data.K;

job_indexes = [];
count = 0;
for k = 1:2
    for i = 1:numel(K)
        for j = 1:K(i)
            count = count + 1;
            job_indexes(count,:) = [k K(i) j];
        end
    end
end
data.job_indexes = job_indexes;

save(data_file, 'data');