function data = prepare_clustering_data

% PASCAL path
opt = globals();
pascal_init;

% load ids
ids = textread(sprintf(VOCopts.imgsetpath, 'trainval'), '%s');
N = numel(ids);

count = 0;
id = [];
cls = [];
imgname = [];
bbox = [];
truncated = [];
difficult = [];
is_flip = [];
for i = 1:N
    disp(ids{i});
    % load annotation
    record = PASreadrecord(sprintf(VOCopts.annopath, ids{i}));
    objects = record.objects;
    
    for j = 1:numel(objects)
        object = objects(j);
        count = count + 1;
        id{count} = ids{i};
        cls{count} = object.class;
        imgname{count} = record.filename;
        bbox(:,count) = object.bbox;
        truncated(count) = object.truncated;
        difficult(count) = object.difficult;
        is_flip(count) = 0;
    end
    
    % add flipped objects
    w = record.imgsize(1);
    for j = 1:numel(objects)
        object = objects(j);
        count = count + 1;
        id{count} = ids{i};
        cls{count} = object.class;
        imgname{count} = record.filename;
        
        % flip the box
        bbox(:,count) = object.bbox;
        oldx1 = object.bbox(1);
        oldx2 = object.bbox(3);        
        bbox(1,count) = w - oldx2 + 1;
        bbox(3,count) = w - oldx1 + 1;
        
        truncated(count) = object.truncated;
        difficult(count) = object.difficult;
        is_flip(count) = 1;
    end
end

data.id = id;
data.cls = cls;
data.imgname = imgname;
data.bbox = bbox;
data.truncated = truncated;
data.difficult = difficult;
data.is_flip = is_flip;