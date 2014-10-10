function show_occlusion_patterns

opt = globals();
pascal_init;
is_flip = 0;
is_save = 1;

% load PASCAL3D+ cad models
fprintf('load CAD models from file\n');
object = load('cads.mat');
cads = object.cads;
classes = cads.classes;
models = cads.models;

ids = textread(sprintf(VOCopts.imgsetpath, 'train'), '%s');
N = numel(ids);

hf = figure;
ind_plot = 1;
mplot = 1;
nplot = 3;
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
        if isempty(cls_index) == 0 && isempty(object.grid) == 0 % && object.occ_per > 0.05 && object.occ_per < 0.95 
            cad_index = object.cad_index;
            cad = models{cls_index}(cad_index);
            
            % show 3D pattern
            subplot(mplot, nplot, ind_plot);
            cla;
            draw_cad(cad, object.grid_origin);
            view(object.azimuth, object.elevation);
%             til = sprintf('%s, object %d, occ=%.2f', ids{i}, j, object.occ_per);
%             title(til);
            ind_plot = ind_plot + 1;
            
            % show 2D pattern
            subplot(mplot, nplot, ind_plot);
            cla;
            pattern = object.pattern;
            im = create_mask_image(pattern);
            imshow(im);
            ind_plot = ind_plot + 1;
            axis on;
            xlabel('x');
            ylabel('y');
            
            % show the image patch
            filename = sprintf(VOCopts.imgpath, ids{i});
            I = imread(filename);
            if object.occ_per > 0
                bbox_gt = [object.x1 object.y1 object.x2 object.y2];
            else
                bbox_gt = object.bbox;
            end
            rect = [bbox_gt(1) bbox_gt(2) bbox_gt(3)-bbox_gt(1) bbox_gt(4)-bbox_gt(2)];
            I1 = imcrop(I, rect);
            subplot(mplot, nplot, ind_plot);
            cla;
            ind_plot = ind_plot + 1;
            imshow(I1);
            
            if is_flip
                % show flipped pattern
                subplot(mplot, nplot, ind_plot);
                cla;
                object_flip = record.objects_flip(j);
                draw_cad(cad, object_flip.grid_origin);
                view(object_flip.azimuth, object_flip.elevation);
                til = sprintf('%s, object %d, occ=%.2f', ids{i}, j, object_flip.occ_per);
                title(til);
                ind_plot = ind_plot + 1;

                % show flipped image patch
                I = I(:, end:-1:1, :);
                bbox = object_flip.bbox;
                rect = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
                I1 = imcrop(I, rect);
                subplot(mplot, nplot, ind_plot);
                cla;
                ind_plot = ind_plot + 1;
                imshow(I1);
            end
            
            if ind_plot > mplot*nplot
                ind_plot = 1;
                if is_save
                    filepath = sprintf('Patterns/%s', classes{cls_index});
                    if exist(filepath, 'dir') == 0
                        unix(['mkdir -p ' filepath]);
                    end
                    filename = fullfile('Patterns', classes{cls_index}, sprintf('%s_%d.png', ids{i}, j));
                    saveas(hf, filename);
                else
                    pause;
                end
            end
        end
    end
end