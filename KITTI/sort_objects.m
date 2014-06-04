% sort objects from large distance to small distance
function index = sort_objects(objects)

num = numel(objects);
dis = zeros(num, 1);

for i = 1:num
    dis(i) = norm(objects(i).t);
end

[~, index] = sort(dis, 'descend');