function show_training_data

is_train = 1;
K = 10;
cache_dir = 'CACHED_DATA_TRAINVAL';

% load ids
object = load('kitti_ids_new.mat');
if is_train
    ids = object.ids_train;
else
    ids = [object.ids_train object.ids_val];
end

% KITTI path
globals;
root_dir = KITTIroot;
if is_train
    data_set = 'training';
else
    data_set = 'testing';
end
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

N = numel(ids);
for i = 1:N
    % read image
    fprintf('%d\n', ids(i));
    file_img = sprintf('%s/%06d.png',image_dir, ids(i));
    I = imread(file_img);

    % read detections
    filename = fullfile(cache_dir, sprintf('%06d.mat', ids(i)));
    object = load(filename);
    Detections = object.Detections;
    Scores = object.Scores;
    Patterns = object.Patterns;
    
    % sort the detection scores
    [~, index] = sort(Scores, 'descend');
    for j = 1:min(numel(index), K)
        ind = index(j);

        bbox_pr = Detections(ind, 1:4);
        bbox = zeros(1,4);
        bbox(1) = max(1, round(bbox_pr(1)));
        bbox(2) = max(1, round(bbox_pr(2)));
        bbox(3) = min(size(I,2), round(bbox_pr(3)));
        bbox(4) = min(size(I,1), round(bbox_pr(4)));      
        
        pattern = Patterns(:,:,ind);
        im = create_mask_image(pattern);
        index_pattern = im == 255;
        im(index_pattern) = I(index_pattern);
        I = uint8(0.1*I + 0.9*im);
    end
    
    imshow(I);
    hold on;
    
    for j = 1:min(numel(index), K)       
        ind = index(j);
        bbox = Detections(ind, 1:4);
        bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
        rectangle('Position', bbox_draw', 'EdgeColor', 'g');
    end    
    
    hold off;
    pause;
end