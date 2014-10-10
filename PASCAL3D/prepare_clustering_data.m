function data = prepare_clustering_data

% PASCAL path
opt = globals();
pascal_init;
ids = textread(sprintf(VOCopts.imgsetpath, 'train'), '%s');
N = numel(ids);

% load PASCAL3D+ cad models
fprintf('load CAD models from file\n');
object = load('cads.mat');
cads = object.cads;
classes = cads.classes;
models_mean = cads.models_mean;

count = 0;
id = [];
cls = [];
cls_ind = [];
imgname = [];
cad_index = [];
bbox = [];
azimuth = [];
elevation = [];
distance = [];
occ_per = [];
trunc_per = [];
difficult = [];
pattern = [];
grid = [];
is_flip = [];
for i = 1:N
    % load annotation
    filename = sprintf('Annotations/%s.mat', ids{i});
    disp(filename);
    object = load(filename);
    record = object.record;
    objects = [record.objects record.objects_flip];
    
    for j = 1:numel(objects)
        object = objects(j);
        cls_index = find(strcmp(object.class, classes) == 1);
        if isempty(cls_index) == 0 && isempty(object.grid) == 0
            count = count + 1;
            id{count} = ids{i};
            cls{count} = object.class;
            cls_ind(count) = cls_index;
            imgname{count} = record.filename;
            cad_index(count) = object.cad_index;
            if object.occ_per > 0
                bbox(:,count) = [object.x1 object.y1 object.x2 object.y2];
            else
                bbox(:,count) = object.bbox;
            end
            azimuth(count) = object.azimuth;
            elevation(count) = object.elevation;
            distance(count) = object.distance;
            occ_per(count) = object.occ_per;
            trunc_per(count) = object.trunc_per;
            difficult(count) = object.difficult;
            pattern{count} = object.pattern;
            grid{count} = object.grid(models_mean{cls_index}.grid == 1);
            is_flip(count) = object.is_flip;
        end
    end
end

data.classes = classes;
data.id = id;
data.cls = cls;
data.cls_ind = cls_ind;
data.imgname = imgname;
data.cad_index = cad_index;
data.bbox = bbox;
data.azimuth = azimuth;
data.elevation = elevation;
data.distance = distance;
data.occ_per = occ_per;
data.trunc_per = trunc_per;
data.difficult = difficult;
data.pattern = pattern;
data.grid = grid;
data.is_flip = is_flip;