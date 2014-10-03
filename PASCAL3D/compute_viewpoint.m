% compute the viewpoints for pascal objects
function compute_viewpoint

opt = globals();
pascal_init;
issave = 0;
pad_size = 100;
use_nonvisible = 1;
clear_nonvisible = 0;

% load PASCAL3D+ cad models
fprintf('load CAD models from file\n');
object = load('cads.mat');
cads = object.cads;
classes = cads.classes;
rescales = cads.rescales;
models = cads.models;

ids = textread(sprintf(VOCopts.imgsetpath, 'trainval'), '%s');

for i = 1:length(ids)
    fprintf('%d %s\n', i, ids{i});
    object = load(sprintf('Annotations/%s.mat', ids{i}));
    record = object.record;
    [azimuth, elevation, azi_co, ele_co, distance, focal, px, py,...
        theta, error, interval_azimuth, interval_elevation, num_anchor, ob_index, tmp]...
        = view_estimator(classes, record, models, use_nonvisible, clear_nonvisible, rescales);
    record = tmp;
    
    for j = 1:length(ob_index)
        record.objects(ob_index(j)).viewpoint.azimuth = azimuth(j);
        record.objects(ob_index(j)).viewpoint.elevation = elevation(j);
        record.objects(ob_index(j)).viewpoint.distance = distance(j);
        record.objects(ob_index(j)).viewpoint.focal = focal(j);
        record.objects(ob_index(j)).viewpoint.px = px(j);
        record.objects(ob_index(j)).viewpoint.py = py(j);
        record.objects(ob_index(j)).viewpoint.theta = theta(j);
        record.objects(ob_index(j)).viewpoint.error = error(j);
        record.objects(ob_index(j)).viewpoint.interval_azimuth = interval_azimuth(j);
        record.objects(ob_index(j)).viewpoint.interval_elevation = interval_elevation(j);
        record.objects(ob_index(j)).viewpoint.num_anchor = num_anchor(j);
        record.objects(ob_index(j)).viewpoint.viewport = 3000;
    end

    if issave == 1
        save(sprintf('Annotations/%s.mat', ids{i}), 'record');
    else
        if numel(find(distance > 0)) < 2
            continue;
        end
        
        hf = figure(1);
        cmap = colormap(jet);        
        
        % show image
        subplot(1, 2, 1);
        filename = sprintf(VOCopts.imgpath, ids{i});
        I = imread(filename);
        [h, w, ~] = size(I);
        mask = ones(h, w, 3);
        mask = padarray(mask, [pad_size pad_size 0]);
        
        mask_object = zeros(h, w);
        mask_object = padarray(mask_object, [pad_size pad_size]);
        
        imshow(I);
        hold on;

        % load annotations
        objects = record.objects;

        % sort objects from large distance to small distance
        index = sort_objects(objects);

        % for all annotated objects do
        is_overlap = 0;
        for j = 1:numel(index)
            obj_idx = index(j);
            % plot 2D bounding box
            object = objects(obj_idx);
            fprintf('%s %d\n', object.class, object.cad_index);
            bbox = object.bbox;
            bbox_draw = [bbox(1) bbox(2) bbox(3)-bbox(1) bbox(4)-bbox(2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g');

            cls_index = find(strcmp(object.class, classes) == 1);
            if isempty(cls_index) == 0
                cad_index = object.cad_index;
                x3d = models{cls_index}(cad_index).vertices * rescales{cls_index}(cad_index);
                x2d = project_3d(x3d, object);
                if isempty(x2d)
                    continue;
                end
                face = models{cls_index}(cad_index).faces;

                index_color = 1 + floor((j-1) * size(cmap,1) / numel(index));
                patch('vertices', x2d, 'faces', face, ...
                    'FaceColor', cmap(index_color,:), 'FaceAlpha', 0.2, 'EdgeColor', 'none');

                x2d = x2d + pad_size;
                vertices = [x2d(face(:,1),2) x2d(face(:,1),1) ...
                            x2d(face(:,2),2) x2d(face(:,2),1) ...
                            x2d(face(:,3),2) x2d(face(:,3),1)];

                BW = mesh_test(vertices, h+2*pad_size, w+2*pad_size);
                
                if sum(sum(mask_object > 0 & BW)) > 0
                    is_overlap = 1;
                end
                mask_object(BW) = obj_idx;

                for k = 1:3
                    tmp = mask(:,:,k);
                    tmp(BW) = cmap(index_color,k);
                    mask(:,:,k) = tmp;
                end
            end
        end
        hold off;

        subplot(1,2,2);
        mask = mask(pad_size+1:h+pad_size, pad_size+1:w+pad_size,:);
        imshow(uint8(255*mask));
        axis off;  
        axis equal;
        
        if is_overlap
            filename = sprintf('Groundtruths/%s.png', ids{i});
            saveas(hf, filename);
        end

%         pause;
    end
end