function show_exemplar_dpms

opt = globals;

% load data
object = load('data.mat');
data = object.data;

% load the mean CAD model
cls = 'car';
filename = sprintf('%s/%s_kitti_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
index = cad.grid == 1;

% cluster centers
files = dir('../Exemplar_DPM/kitti_train_ap_125/*final*');
N = numel(files);
centers = zeros(1, N);
for i = 1:N
    filename = files(i).name;
    k = strfind(filename, '_');
    centers(i) = str2double(filename(k(1)+1:k(2)-1));
end
centers = sort(centers);

% sort centers according to azimuth
azimuth = data.azimuth(centers);
[~, order] = sort(azimuth);
centers = centers(order);

figure;
ind_plot = 1;
for i = 1:N
    % show center
    ind = centers(i);
    grid = data.grid(:,ind);
    visibility_grid = zeros(size(cad.grid));
    visibility_grid(index) = grid;
    subplot(4, 4, ind_plot);
    ind_plot = ind_plot + 1;
    draw_cad(cad, visibility_grid);
    view(data.azimuth(ind), data.elevation(ind));
%     til = sprintf('%d: trunc %.2f', ind, data.truncation(ind));
%     title(til);
    
    % show exemplar DPM
    filename = sprintf('../Exemplar_DPM/kitti_train_ap_125/%s_%d_final.mat', cls, ind);
    if exist(filename, 'file')
        object = load(filename);
        model = object.model;
        colormap('default');
        visualizemodel(model, 4, 4, ind_plot);
        ind_plot = ind_plot + 3;
    end
    
    if ind_plot > 16
        ind_plot = 1;
        pause;
    end
end