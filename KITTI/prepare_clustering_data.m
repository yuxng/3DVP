function data = prepare_clustering_data

% parameters
min_occlusion = 0.05;
max_occlusion = 0.95;

% load mean model
cls = 'car';
filename = sprintf('../Geometry/%s_mean.mat', cls);
object = load(filename);
cad = object.(cls);
index = cad.grid == 1;

% annotations
path_ann = 'Annotations';
files = dir(fullfile(path_ann, '*.mat'));
N = numel(files);

count = 0;
azimuth = [];
elevation = [];
distance = [];
occ_per = [];
grid = [];
for i = 1:N
    % load annotation
    filename = fullfile(path_ann, files(i).name);
    disp(filename);
    object = load(filename);
    record = object.record;
    objects = record.objects;
    
    for j = 1:numel(objects)
        object = objects(j);
        if strcmp(object.type, 'Car') == 1 && object.occ_per > min_occlusion ...
                && object.occ_per < max_occlusion
            count = count + 1;
            azimuth(count) = object.azimuth;
            elevation(count) = object.elevation;
            distance(count) = object.distance;
            occ_per(count) = object.occ_per;
            grid(:,count) = object.grid(index);
        end
    end
end

data.min_occlusion = min_occlusion;
data.max_occlusion = max_occlusion;
data.azimuth = azimuth;
data.elevation = elevation;
data.distance = distance;
data.occ_per = occ_per;
data.grid = grid;