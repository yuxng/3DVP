function data = prepare_clustering_data

% KITTI path
opt = globals();
root_dir = opt.path_kitti_root;
data_set = 'training';
calib_dir = fullfile(root_dir,[data_set '/calib']);

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
truncation = [];
pattern = [];
grid = [];
translation = [];
for i = 1:N%3740
    img_idx = i - 1;
    
    % load the velo_to_cam matrix
    R0_rect = readCalibration(calib_dir, img_idx, 4);
    tmp = R0_rect';
    tmp = tmp(1:9);
    tmp = reshape(tmp, 3, 3);
    tmp = tmp';
    Pv2c = readCalibration(calib_dir, img_idx, 5);
    Pv2c = tmp * Pv2c;
    Pv2c = [Pv2c; 0 0 0 1];    
    
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
            truncation(count) = object.truncation;
            pattern{count} = object.pattern;
            grid(:,count) = object.grid(index);            
            % transform to velodyne space
            X = [object.t'; 1];
            X = Pv2c\X;
            X(4) = [];
            translation(:,count) = X;
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
data.truncation = truncation;
data.pattern = pattern;
data.grid = grid;
data.translation = translation;