function data = prepare_clustering_data

is_train = 1;

% KITTI path
opt = globals();
root_dir = opt.path_kitti_root;
data_set = 'training';
calib_dir = fullfile(root_dir,[data_set '/calib']);

% load mean model
cls = 'car';
filename = sprintf('../Geometry/%s_kitti_mean.mat', cls);
object = load(filename);
cad = object.(cls);
index = cad.grid == 1;

% load ids
object = load('kitti_ids_new.mat');
if is_train
    ids = object.ids_train;
else
    ids = [object.ids_train object.ids_val];
end

count = 0;
id = [];
imgname = [];
bbox = [];
l = [];
h = [];
w = [];
alpha = [];
azimuth = [];
elevation = [];
distance = [];
occ_per = [];
truncation = [];
occlusion = [];
pattern = [];
grid = [];
translation = [];
is_flip = [];
cad_index = [];

for i = 1:numel(ids)
    img_idx = ids(i);
    
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
    filename = fullfile('Annotations', sprintf('%06d.mat', img_idx));
    disp(filename);
    object = load(filename);
    record = object.record;
    objects = [record.objects record.objects_flip];
    
    for j = 1:numel(objects)
        object = objects(j);
        if strcmp(object.type, 'Car') == 1 && isempty(object.grid) == 0
            count = count + 1;
            id(count) = img_idx;
            imgname{count} = record.filename;
            bbox(:,count) = [object.x1; object.y1; object.x2; object.y2];
            l(count) = object.l;
            h(count) = object.h;
            w(count) = object.w;
            alpha(count) = object.alpha;
            azimuth(count) = object.azimuth;
            elevation(count) = object.elevation;
            distance(count) = object.distance;
            occ_per(count) = object.occ_per;
            truncation(count) = object.truncation;
            occlusion(count) = object.occlusion;
            pattern{count} = object.pattern;
            grid(:,count) = object.grid(index);            
            % transform to velodyne space
            X = [object.t'; 1];
            X = Pv2c\X;
            X(4) = [];
            translation(:,count) = X;
            is_flip(count) = object.is_flip;
            cad_index(count) = object.cad_index;
        end
    end
end

data.id = id;
data.imgname = imgname;
data.bbox = bbox;
data.l = l;
data.h = h;
data.w = w;
data.alpha = alpha;
data.azimuth = azimuth;
data.elevation = elevation;
data.distance = distance;
data.occ_per = occ_per;
data.truncation = truncation;
data.occlusion = occlusion;
data.pattern = pattern;
data.grid = grid;
data.translation = translation;
data.is_flip = is_flip;
data.cad_index = cad_index;