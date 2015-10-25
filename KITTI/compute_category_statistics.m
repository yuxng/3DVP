function compute_category_statistics

classes = {'car', 'pedestrian', 'cyclist'};

for i = 1:numel(classes)
    cls = classes{i};
    
    if strcmp(cls, 'car') == 1
        filename = 'data_kitti.mat';
    else
        filename = sprintf('data_kitti_%s.mat', cls);
    end
    
    object = load(filename);
    data = object.data;
    
    height = data.bbox(4,:) - data.bbox(2,:) + 1;
    occlusion = data.occlusion;
    truncation = data.truncation;
    flag = height > 25 & occlusion < 3 & truncation < 0.5;
    fprintf('%s: %d, %d examples\n', cls, numel(data.id)/2, sum(flag)/2);
end