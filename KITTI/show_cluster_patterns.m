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
idx = data.idx_kmeans;

% load the mean CAD model
cls = 'car';
filename = sprintf('%s/%s_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
index = cad.grid == 1;

% cluster centers
if nargin < 1
    centers = unique(idx);
    centers(centers == -1) = [];
else
    centers = cid;
end
N = numel(centers);

figure;
nplot = 8;
mplot = 8;
for i = 1:N
    ind = centers(i);

    bbox = data.bbox(:, idx == ind);
    % plot of center locations of bounding boxes
    x = (bbox(1,:) + bbox(3,:)) / 2;
    y = (bbox(2,:) + bbox(4,:)) / 2;

    h = subplot(nplot, mplot, 1:mplot/2);
    plot(x, y, 'o');
    set(h, 'Ydir', 'reverse');
    axis equal;
    axis([1 1242 1 375]);
    xlabel('x');
    ylabel('y');
    title('location');
    
    % plot of widths and heights of bounding boxes
    w = bbox(3,:) - bbox(1,:);
    h = bbox(4,:) - bbox(2,:);

    subplot(nplot, mplot, mplot/2+1:mplot);
    plot(w, h, 'o');
    axis equal;
    axis([1 1242 1 375]);
    xlabel('w');
    ylabel('h');
    title('width and height');    
    
    ind_plot = mplot + 1;
    % show center
    grid = data.grid(:,ind);
    visibility_grid = zeros(size(cad.grid));
    visibility_grid(index) = grid;
    subplot(nplot, mplot, ind_plot);
    ind_plot = ind_plot + 1;
    draw_cad(cad, visibility_grid);
    view(data.azimuth(ind), data.elevation(ind));
    
    % show the image patch
    filename = fullfile(image_dir, data.imgname{ind});
    I = imread(filename);
    bbox = data.bbox(:,ind);
    rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
    I1 = imcrop(I, rect);
    subplot(nplot, mplot, ind_plot);
    ind_plot = ind_plot + 1;
    imshow(I1);
    til = sprintf('w:%d, h:%d', size(I1,2), size(I1,1));
    title(til);
    
    % show several members
    member = find(idx == ind);
    member(member == ind) = [];
    num = numel(member);
    for j = 1:min((nplot-1)*mplot/2-1, num)
        ind = member(j);
        grid = data.grid(:,ind);
        visibility_grid = zeros(size(cad.grid));
        visibility_grid(index) = grid;
        subplot(nplot, mplot, ind_plot);
        ind_plot = ind_plot + 1;
        draw_cad(cad, visibility_grid);
        view(data.azimuth(ind), data.elevation(ind));
        
        % show the image patch
        filename = fullfile(image_dir, data.imgname{ind});
        I = imread(filename);
        bbox = data.bbox(:,ind);
        rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
        I1 = imcrop(I, rect);
        subplot(nplot, mplot, ind_plot);
        ind_plot = ind_plot + 1;
        imshow(I1);
        til = sprintf('w:%d, h:%d', size(I1,2), size(I1,1));
        title(til);        
    end
    for j = ind_plot:nplot*mplot
        subplot(nplot, mplot, j);
        cla;
        title('');
    end
    pause;
end