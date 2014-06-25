function show_cluster_patterns(idx)

opt = globals;

% load data
object = load('data.mat');
data = object.data;

% load the mean CAD model
cls = 'car';
filename = sprintf('%s/%s_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
index = cad.grid == 1;

% cluster centers
centers = unique(idx);
N = numel(centers);

figure;
for i = 1:N
    % show center
    ind = centers(i);
    grid = data.grid(:,ind);
    visibility_grid = zeros(size(cad.grid));
    visibility_grid(index) = grid;
    subplot(4, 4, 1);
    draw_cad(cad, visibility_grid);
    view(data.azimuth(ind), data.elevation(ind));    
    
    % show several members
    member = find(idx == ind);
    member(member == ind) = [];
    num = numel(member);
    for j = 1:min(15, num)
        ind = member(j);
        grid = data.grid(:,ind);
        visibility_grid = zeros(size(cad.grid));
        visibility_grid(index) = grid;
        subplot(4, 4, 1+j);
        draw_cad(cad, visibility_grid);
        view(data.azimuth(ind), data.elevation(ind));        
    end
    pause;
end