% create annotations with occlusion masks for PASCAL3D dataset
function create_annotations

opt = globals();
pascal_init;
classes = {'aeroplane', 'bicycle', 'boat', 'bottle', 'bus', 'car', 'chair', ...
    'diningtable', 'motorbike', 'sofa', 'train', 'tvmonitor'};
is_pascal = 0;

% pascal voc 2012
if is_pascal
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
else
    % imagenet
    for k = 1:numel(classes)
        cls = classes{k};
        ids_train = textread(sprintf(opt.path_set_imagenet_train, cls), '%s');
        ids_val = textread(sprintf(opt.path_set_imagenet_val, cls), '%s');
        ids = [ids_train; ids_val];
        for i = 1:numel(ids)
            disp(ids{i});
            % load annotation
            filename_src = [sprintf(opt.path_ann_imagenet, cls) '/' ids{i} '.mat'];
            object = load(filename_src);
            record = object.record;
            record.cls = cls;
            % save annotation
            filename_dst = sprintf('Annotations/%s.mat', ids{i});
            save(filename_dst, 'record');
        end
    end
end