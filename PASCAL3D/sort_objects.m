% sort objects from large distance to small distance
function index = sort_objects(objects)

num = numel(objects);
dis = zeros(num, 1);

for i = 1:num
    if isfield(objects(i), 'viewpoint') && ~isempty(objects(i).viewpoint)
        dis(i) = objects(i).viewpoint.distance;
    end
end

[~, index] = sort(dis, 'descend');