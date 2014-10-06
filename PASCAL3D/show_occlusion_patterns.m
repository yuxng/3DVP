function show_occlusion_patterns

opt = globals();
pascal_init;

% load PASCAL3D+ cad models
fprintf('load CAD models from file\n');
object = load('cads.mat');
cads = object.cads;
classes = cads.classes;
models_mean = cads.models_mean;

ids = textread(sprintf(VOCopts.imgsetpath, 'train'), '%s');
N = numel(ids);

figure;
ind_plot = 1;
for i = 1:N
    % load annotation
    filename = sprintf('Annotations/%s.mat', ids{i});
    disp(filename);
    object = load(filename);
    record = object.record;
    objects = record.objects;
    
    for j = 1:numel(objects)
        object = objects(j);
        cls_index = find(strcmp(object.class, classes) == 1);
        if isempty(cls_index) == 0 && isempty(object.grid) == 0 && cls_index == 6 % && object.occ_per > 0.05 && object.occ_per < 0.95 
            cad = models_mean{cls_index};
            
            % show pattern
            subplot(4, 4, ind_plot);
            cla;
            draw_cad(cad, object.grid);
            view(object.azimuth, object.elevation);
            til = sprintf('%s, object %d, occ=%.2f', ids{i}, j, object.occ_per);
            title(til);
            ind_plot = ind_plot + 1;
            
            % show the image patch
            filename = sprintf(VOCopts.imgpath, ids{i});
            I = imread(filename);
            bbox = object.bbox;
            rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            I1 = imcrop(I, rect);
            subplot(4, 4, ind_plot);
            cla;
            ind_plot = ind_plot + 1;
            imshow(I1);
            
            % show flipped pattern
            subplot(4, 4, ind_plot);
            cla;
            object_flip = record.objects_flip(j);
            draw_cad(cad, object_flip.grid);
            view(object_flip.azimuth, object_flip.elevation);
            til = sprintf('%s, object %d, occ=%.2f', ids{i}, j, object_flip.occ_per);
            title(til);
            ind_plot = ind_plot + 1;
            
            % show flipped image patch
            I = I(:, end:-1:1, :);
            bbox = object_flip.bbox;
            rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            I1 = imcrop(I, rect);
            subplot(4, 4, ind_plot);
            cla;
            ind_plot = ind_plot + 1;
            imshow(I1);            
            
            if ind_plot > 16
                ind_plot = 1;
                pause;
            end
        end
    end
end