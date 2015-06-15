function show_cluster_patterns_triplet(cid)

is_save = 1;

opt = globals;

% get image directory
root_dir = opt.path_kitti_root;
data_set = 'training';
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% load data
object = load('data_kitti_new.mat');
data = object.data;
idx = data.idx_ap;

% load the CAD models
cls = 'car';
filename = sprintf('%s/%s_kitti.mat', opt.path_slm_geometry, cls);
object = load(filename);
cads = object.(cls);

% cluster centers
if nargin < 1
    centers = unique(idx);
    centers(centers == -1) = [];
else
    centers = cid;
end
N = numel(centers);

azimuth = data.azimuth(centers);
[~, ind] = sort(azimuth);
centers = centers(ind);

% hf = figure('units','normalized','outerposition',[0 0 0.5 1]);
hf = figure;
nplot = 5;
mplot = 6;

for i = 1:N
    disp(i);
    ind_plot = 1;
    ind = centers(i);
    
    % show the image patch
    filename = fullfile(image_dir, data.imgname{ind});
    I = imread(filename);
    if data.is_flip(ind) == 1
        I = I(:, end:-1:1, :);
    end    
    bbox = data.bbox(:,ind);
    rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
    I1 = imcrop(I, rect);
    subplot(nplot, mplot, ind_plot);
    cla;
    ind_plot = ind_plot + 1;
    imshow(I1);
%     til = sprintf('w:%d, h:%d', size(I1,2), size(I1,1));
%     title(til);

    % show 2D pattern
    subplot(nplot, mplot, ind_plot);
    cla;
    pattern = data.pattern{ind};
    im = create_mask_image(pattern);
    imshow(im);
    ind_plot = ind_plot + 1;
    axis off;
    
    % show center
    grid = data.grid_origin{ind};
    cad = cads(data.cad_index(ind));
    index = cad.grid == 1;
    visibility_grid = zeros(size(cad.grid));
    visibility_grid(index) = grid;
    subplot(nplot, mplot, ind_plot);
    cla;
    ind_plot = ind_plot + 1;
    draw_cad(cad, visibility_grid);
    view(data.azimuth(ind), data.elevation(ind));    
    
    % show several members
    member = find(idx == ind);
    member(member == ind) = [];
    num = numel(member);
    fprintf('%d examples\n', num+1);
    for j = 1:min(nplot*mplot/3-1, num)
        ind = member(j);
        
        % show the image patch
        filename = fullfile(image_dir, data.imgname{ind});
        I = imread(filename);
        if data.is_flip(ind) == 1
            I = I(:, end:-1:1, :);
        end        
        bbox = data.bbox(:,ind);
        rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
        I1 = imcrop(I, rect);
        subplot(nplot, mplot, ind_plot);
        cla;
        ind_plot = ind_plot + 1;
        imshow(I1);
%         til = sprintf('w:%d, h:%d', size(I1,2), size(I1,1));
%         title(til);

        % show 2D pattern
        subplot(nplot, mplot, ind_plot);
        cla;
        pattern = data.pattern{ind};
        im = create_mask_image(pattern);
        imshow(im);
        ind_plot = ind_plot + 1;
        axis off;
        
        grid = data.grid_origin{ind};
        cad = cads(data.cad_index(ind));
        index = cad.grid == 1;        
        visibility_grid = zeros(size(cad.grid));
        visibility_grid(index) = grid;
        subplot(nplot, mplot, ind_plot);
        cla;
        ind_plot = ind_plot + 1;
        draw_cad(cad, visibility_grid);
        view(data.azimuth(ind), data.elevation(ind));        
    end
    for j = ind_plot:nplot*mplot
        subplot(nplot, mplot, j);
        cla;
        title('');
    end
    if is_save
        filename = sprintf('Clusters/%03d.png', i);
        hgexport(hf, filename, hgexport('factorystyle'), 'Format', 'png');
    else
        pause;
    end
end