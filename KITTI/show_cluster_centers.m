function show_cluster_centers

opt = globals;

% load data
object = load('data.mat');
data = object.data;
idx = data.idx_ap;

% load the mean CAD model
cls = 'car';
filename = sprintf('%s/%s_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
index = cad.grid == 1;

% cluster centers
centers = unique(idx);
N = numel(centers);

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
    subplot(5, 5, ind_plot);
    ind_plot = ind_plot + 1;
    draw_cad(cad, visibility_grid);
    view(data.azimuth(ind), data.elevation(ind));
    num = numel(find(idx == ind));
    til = sprintf('%d: trunc %.2f, %d examples', ind, data.truncation(ind), num);
    title(til);
    
    % show exemplar DPM
%     filename = sprintf('../Exemplar_DPM/kitti_train/%s_%d_final.mat', cls, ind);
%     if exist(filename, 'file')
%         object = load(filename);
%         model = object.model;
%         visualizemodel(model, 4, 4, ind_plot);
%         ind_plot = ind_plot + 3;
%     end
    
    if ind_plot > 25
        ind_plot = 1;
        pause;
    end
end