function show_exemplar_mean_gradients

opt = globals;
is_train = 0;
is_save = 0;

% load data
if is_train
    object = load('data.mat');
else
    object = load('data_kitti.mat');
end
data = object.data;
idx = data.idx_ap;

% load the mean CAD model
cls = 'car';
filename = sprintf('%s/%s_kitti_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
index = cad.grid == 1;

% cluster centers
centers = unique(idx);
centers(centers == -1) = [];
N = numel(centers);

% sort centers according to azimuth
azimuth = data.azimuth(centers);
[~, order] = sort(azimuth);
centers = centers(order);

hf = figure(1);
ind_plot = 1;
mplot = 6;
nplot = 12;
for i = 1:N
    % show center
    ind = centers(i);
    grid = data.grid(:,ind);
    visibility_grid = zeros(size(cad.grid));
    visibility_grid(index) = grid;
    subplot(mplot, nplot, ind_plot);
    ind_plot = ind_plot + 1;
    draw_cad(cad, visibility_grid);
    view(data.azimuth(ind), data.elevation(ind));
    xlabel('');
    ylabel('');
    zlabel('');
    axis off;
%     til = sprintf('%d: trunc %.2f', ind, data.truncation(ind));
%     title(til);
    
    % show exemplar DPM
    [mimages, gimages] = draw_mean_image(data, ind);
    subplot(mplot, nplot, ind_plot);
    ind_plot = ind_plot + 1;    
    imshow(mimages);    
    
    subplot(mplot, nplot, ind_plot);
    ind_plot = ind_plot + 1;    
    imshow(gimages);
    
    if ind_plot > mplot * nplot
        ind_plot = 1;
        if is_save
            if is_train
                filename = sprintf('Clusters/center_ap_%d_train.png', i);
            else
                filename = sprintf('Clusters/center_ap_%d_test.png', i);
            end
            saveas(hf, filename);
        else
            pause;
        end
    end
end