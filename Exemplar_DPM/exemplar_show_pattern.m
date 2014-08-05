function exemplar_show_pattern(data, cad, cid)

globals;
addpath(genpath('../Geometry'));

% get image directory
root_dir = KITTIroot;
data_set = 'training';
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

index = cad.grid == 1;

h = figure;
cla;

ind_plot = 1;
% show center
ind = cid;
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

% show several members
member = find(data.idx == ind);
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
end

set(h, 'Units', 'normalized', 'Position', [0,0,1,1]);