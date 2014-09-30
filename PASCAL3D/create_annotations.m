% create annotations with occlusion masks for PASCAL3D dataset
function create_annotations

opt = globals();
pascal_init;
classes = {'aeroplane', 'bicycle', 'boat', 'bottle', 'bus', 'car', 'chair', ...
    'diningtable', 'motorbike', 'sofa', 'train', 'tvmonitor'};

ids = textread(sprintf(VOCopts.imgsetpath, 'trainval'), '%s');
for i = 1:length(ids);
    disp(ids{i});
    record = PASreadrecord(sprintf(VOCopts.annopath, ids{i}));
    n = numel(record.objects);
    for j = 1:n
        flag = strcmp(record.objects(j).class, classes);
        index = find(flag == 1);
        if isempty(index) == 0
            cls = classes{index};
            filename = sprintf([opt.path_ann_pascal '/%s.mat'], cls, ids{i});
            object = load(filename);
            record.objects(j).anchors = object.record.objects(j).anchors;
            record.objects(j).viewpoint = object.record.objects(j).viewpoint;
            record.objects(j).cad_index = object.record.objects(j).cad_index;
        end
    end
    
    % save annotation
    filename = sprintf('Annotations/%s.mat', ids{i});
    save(filename, 'record');    
end