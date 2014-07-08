function show_cluster_patterns(cid)

opt = globals;

% get image directory
root_dir = opt.path_kitti_root;
data_set = 'training';
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% load data
object = load('data.mat');
data = object.data;
idx = data.idx;

% load the mean CAD model
cls = 'car';
filename = sprintf('%s/%s_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
index = cad.grid == 1;

% cluster centers
if nargin < 1
    centers = unique(idx);
else
    centers = cid;
end
N = numel(centers);

figure;
for i = 1:N
    ind_plot = 1;
    % show center
    ind = centers(i);
    grid = data.grid(:,ind);
    visibility_grid = zeros(size(cad.grid));
    visibility_grid(index) = grid;
    subplot(4, 8, ind_plot);
    ind_plot = ind_plot + 1;
    draw_cad(cad, visibility_grid);
    view(data.azimuth(ind), data.elevation(ind));
    
    % show the image patch
    filename = fullfile(image_dir, data.imgname{ind});
    I = imread(filename);
    bbox = data.bbox(:,ind);
    rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
    I1 = imcrop(I, rect);
    subplot(4, 8, ind_plot);
    ind_plot = ind_plot + 1;
    imshow(I1);
    til = sprintf('w:%d, h:%d', size(I1,2), size(I1,1));
    title(til);
    
    % show several members
    member = find(idx == ind);
    member(member == ind) = [];
    num = numel(member);
    for j = 1:min(15, num)
        ind = member(j);
        grid = data.grid(:,ind);
        visibility_grid = zeros(size(cad.grid));
        visibility_grid(index) = grid;
        subplot(4, 8, ind_plot);
        ind_plot = ind_plot + 1;
        draw_cad(cad, visibility_grid);
        view(data.azimuth(ind), data.elevation(ind));
        
        % show the image patch
        filename = fullfile(image_dir, data.imgname{ind});
        I = imread(filename);
        bbox = data.bbox(:,ind);
        rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
        I1 = imcrop(I, rect);
        subplot(4, 8, ind_plot);
        ind_plot = ind_plot + 1;
        imshow(I1);
        til = sprintf('w:%d, h:%d', size(I1,2), size(I1,1));
        title(til);        
    end
    pause;
end