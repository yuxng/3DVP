function add_job_indexes_kmeans

data_file = 'data_all.mat';
object = load(data_file);
data = object.data;
classes = data.classes;
K = data.K;

job_indexes = [];
count = 0;
for k = 1:2
    for i = 1:numel(classes)
        for j = 1:numel(K)
            count = count + 1;
            job_indexes(count,:) = [k i j];
        end
    end
end
data.job_indexes = job_indexes;

save(data_file, 'data', '-v7.3');