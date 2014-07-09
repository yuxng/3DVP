function kitti_test(cls, cid)

% load model
model_name = sprintf('kitti_train/%s_%d_final.mat', cls, cid);
object = load(model_name);
model = object.model;
model.thresh = min(-1, model.thresh);

% KITTI path
globals;
root_dir = KITTIroot;
data_set = 'training';

% get sub-directories
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir,[data_set '/image_' num2str(cam)]); 

% get test image ids
object = load('kitti_ids.mat');
ids_train = object.ids_train;
ids_val = object.ids_val;
ids = [ids_train ids_val];

filename = sprintf('kitti_train/%s_%d_test.mat', cls, cid);

% run detector in each image
try
    load(filename);
catch
    N = numel(ids);
    parfor i = 1:N
        fprintf('%s: center %d: %d/%d\n', cls, cid, i, N);
        img_idx = ids_val(i);
        file_img = sprintf('%s/%06d.png', image_dir, img_idx);
        im = imread(file_img);
        [dets, boxes] = imgdetect(im, model, model.thresh);
    
        if ~isempty(boxes)
            boxes = reduceboxes(model, boxes);
            [dets, boxes] = clipboxes(im, dets, boxes);
            % without non-maximum suppression
            boxes1{i} = dets(:, [1:4 end]);
            parts1{i} = boxes;
        else
            boxes1{i} = [];
            parts1{i} = [];
        end
    end  
    save(filename, 'boxes1', 'parts1');
end