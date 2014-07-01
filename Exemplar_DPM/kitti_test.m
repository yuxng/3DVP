function kitti_test(cls)

% matlabpool open

model_name = sprintf('data/%s_half.mat', cls);
object = load(model_name);
model = object.model;
model.thresh = min(-1.1, model.thresh);

% KITTI path
globals;
root_dir = KITTIroot;
data_set = 'training';

% get sub-directories
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir,[data_set '/image_' num2str(cam)]); 

% get test image ids
object = load('kitti_ids.mat');
ids_val = object.ids_val;

N = numel(ids_val);
dets = cell(N, 1);
% parfor i = 1:N
for i = 1:N
    fprintf('%s: %d/%d\n', cls, i, N);
    img_idx = ids_val(i);
    file_img = sprintf('%s/%06d.png', image_dir, img_idx);
    I = imread(file_img);
    det = process(I, model, model.thresh);
    dets{i} = det;
end

filename = sprintf('data/%s_test.mat', cls);
save(filename, 'dets');

% matlabpool close