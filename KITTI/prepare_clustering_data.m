function data = prepare_clustering_data

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
imgname = [];
bbox = [];
l = [];
h = [];
w = [];
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
        if strcmp(object.type, 'Car') == 1 && isempty(object.grid) == 0
            count = count + 1;
            imgname{count} = record.filename;
            bbox(:,count) = [object.x1; object.y1; object.x2; object.y2];
            l(count) = object.l;
            h(count) = object.h;
            w(count) = object.w;
            azimuth(count) = object.azimuth;
            elevation(count) = object.elevation;
            distance(count) = object.distance;
            occ_per(count) = object.occ_per;
            grid(:,count) = object.grid(index);
        end
    end
end

data.imgname = imgname;
data.bbox = bbox;
data.l = l;
data.h = h;
data.w = w;
data.azimuth = azimuth;
data.elevation = elevation;
data.distance = distance;
data.occ_per = occ_per;
data.grid = grid;