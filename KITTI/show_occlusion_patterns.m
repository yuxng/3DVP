function show_occlusion_patterns(idx)

% load occlusion patterns
object = load('occlusion_patterns.mat');
grid = object.grid;
N = size(grid, 1);

if nargin < 1
    idx = 1:N;
else
    idx = unique(idx);
end

% load the mean CAD model
cls = 'car';
filename = sprintf('../Geometry/%s_mean.mat', cls);
object = load(filename);
cad = object.(cls);

% define the azimuth and elevation for visibility testing
a = 0:45:315;
e = 15;
vnum = numel(a);
per = size(vnum, 1);
visibility_grid = cell(vnum, 1);
num_total = zeros(vnum, 1);
for i = 1:vnum
    visibility_grid{i} = check_visibility(cad, a(i), e);
    tmp = visibility_grid{i} == 1;
    visibility_grid{i} = reshape(tmp, 1, numel(tmp));
    num_total(i) = sum(visibility_grid{i});
end

figure;
for i = 1:numel(idx)
    if i ~= 1 && mod(i-1, 16) == 0
        pause;
    end
    ind = mod(i-1, 16)+1;
    
    pattern = grid(idx(i),:);
    % find the azimuth to show the pattern
    for j = 1:vnum
        num_occluded = sum(visibility_grid{j} & pattern);
        per(j) = num_occluded / num_total(j);
    end
    tmp = abs(per - 0.5);
    [~, index] = min(tmp);
    
    subplot(4, 4, ind);
    draw_cad(cad, pattern);
    view(a(index), e);
end