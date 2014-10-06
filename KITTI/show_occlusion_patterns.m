function show_occlusion_patterns

% annotations
path_ann = 'Annotations';
files = dir(fullfile(path_ann, '*.mat'));
N = numel(files);

% load the mean CAD model
cls = 'car';
filename = sprintf('../Geometry/%s_kitti_mean.mat', cls);
object = load(filename);
cad = object.(cls);

figure;
ind_plot = 1;
for i = 1:N
    % load annotation
    filename = fullfile(path_ann, files(i).name);
    object = load(filename);
    record = object.record;
    objects = record.objects;
    
    for j = 1:numel(objects)
        object = objects(j);
        if strcmp(object.type, 'Car') == 1% && object.occ_per > 0.05 && object.occ_per < 0.95 
            subplot(4, 4, ind_plot);
            draw_cad(cad, object.grid);
            view(object.azimuth, object.elevation);
            til = sprintf('%s, object %d, occ=%.2f', files(i).name, j, object.occ_per);
            title(til);
            ind_plot = ind_plot + 1;
            if ind_plot > 16
                ind_plot = 1;
                pause;
            end
        end
    end
end