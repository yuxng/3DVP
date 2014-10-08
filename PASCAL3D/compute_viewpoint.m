% compute the viewpoints for pascal objects
function compute_viewpoint

matlabpool open;

opt = globals();
pascal_init;
opt.VOCopts = VOCopts;
pad_size = 1000;
use_nonvisible = 1;
clear_nonvisible = 0;

% load PASCAL3D+ cad models
fprintf('load CAD models from file\n');
object = load('cads.mat');
cads = object.cads;
classes = cads.classes;
rescales = cads.rescales;
models = cads.models;
models_mean = cads.models_mean;

ids = textread(sprintf(VOCopts.imgsetpath, 'trainval'), '%s');

parfor i = 1:length(ids)
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

    % read image
    filename = sprintf(opt.VOCopts.imgpath, ids{i});
    I = imread(filename);
    [h, w, ~] = size(I);
    % occlusion mask
    mask = zeros(h, w);
    mask = padarray(mask, [pad_size pad_size]);

    % load annotations
    objects = record.objects;

    % sort objects from large distance to small distance
    index = sort_objects(objects);

    % for all annotated objects do
    num = numel(index);
    BWs = cell(num, 1);    
    for j = 1:num
        obj_idx = index(j);
        object = objects(obj_idx);

        cls_index = find(strcmp(object.class, classes) == 1);
        if isempty(cls_index) == 0
            cad_index = object.cad_index;
            x3d = models{cls_index}(cad_index).vertices * rescales{cls_index}(cad_index);
            x2d = project_3d(x3d, object);
            if isempty(x2d)
                continue;
            end
            face = models{cls_index}(cad_index).faces;
            
%             flag = min(x2d(:,1)) < 0 & max(x2d(:,1)) > w;
%             if flag == 1
%                 continue;
%             end            

            x2d = x2d + pad_size;
            vertices = [x2d(face(:,1),2) x2d(face(:,1),1) ...
                        x2d(face(:,2),2) x2d(face(:,2),1) ...
                        x2d(face(:,3),2) x2d(face(:,3),1)];

            BWs{obj_idx} = mesh_test(vertices, h+2*pad_size, w+2*pad_size);

            mask(BWs{obj_idx}) = obj_idx;
        end
    end

    mask = mask(pad_size+1:h+pad_size, pad_size+1:w+pad_size);
    mask = padarray(mask, [pad_size pad_size], -1);
    record.pad_size = pad_size;
    record.mask = mask;

    % create occlusion patterns
    index_object = index;
    for j = 1:num
        if isempty(BWs{j}) == 1
            objects(j).is_flip = 0;
            objects(j).pattern = [];
            objects(j).occ_per = 0;
            objects(j).trunc_per = 0;
            objects(j).grid = [];
            continue;
        end        
        objects(j).is_flip = 0;
        
        azimuth = objects(j).viewpoint.azimuth;
        elevation = objects(j).viewpoint.elevation;
        distance = objects(j).viewpoint.distance;
          
        objects(j).azimuth = azimuth;
        objects(j).elevation = elevation;
        objects(j).distance = distance;      

        pattern = uint8(BWs{j});  % 1 visible
        pattern(mask > 0 & mask ~= j & BWs{j}) = 2;  % occluded
        pattern(mask == -1 & BWs{j}) = 3;  % truncated
        [x, y] = find(pattern > 0);
        pattern = pattern(min(x):max(x), min(y):max(y));
        objects(j).pattern = pattern;

        % compute occlusion percentage
        occ = numel(find(pattern == 2)) / numel(find(pattern > 0));
        objects(j).occ_per = occ;

        % compute truncation percentage
        trunc = numel(find(pattern == 3)) / numel(find(pattern > 0));
        objects(j).trunc_per = trunc;

        % 3D occlusion mask from mean shape
        cls_index = strcmp(objects(j).class, classes) == 1;
        cad_index = objects(j).cad_index;  
        [visibility_grid, visibility_ind] = check_visibility(models_mean{cls_index}, azimuth, elevation);

        % check the occlusion status of visible voxels
        index = find(visibility_ind == 1);
        x3d = models_mean{cls_index}.x3d(index,:) * rescales{cls_index}(cad_index);
        x2d = project_3d(x3d, objects(j));
        x2d = x2d + pad_size;
        occludee = find(index_object == j);
        for k = 1:numel(index)
            x = round(x2d(k,1));
            y = round(x2d(k,2));
            ind = models_mean{cls_index}.ind(index(k),:);
            if x > pad_size && x <= size(mask,2)-pad_size && y > pad_size && y <= size(mask,1)-pad_size
                if mask(y,x) > 0 && mask(y,x) ~= j % occluded by other objects
                    occluder = find(index_object == mask(y,x));
                    if occluder > occludee
                        visibility_grid(ind(1), ind(2), ind(3)) = 2;
                    end
                end
            else
                visibility_grid(ind(1), ind(2), ind(3)) = 3;  % truncated
            end
        end
        objects(j).grid = visibility_grid;
        
        % 3D occlusion mask from the original shape
        [visibility_grid, visibility_ind] = check_visibility(models{cls_index}(cad_index), azimuth, elevation);

        % check the occlusion status of visible voxels
        index = find(visibility_ind == 1);
        x3d = models{cls_index}(cad_index).x3d(index,:) * rescales{cls_index}(cad_index);
        x2d = project_3d(x3d, objects(j));
        x2d = x2d + pad_size;
        occludee = find(index_object == j);
        for k = 1:numel(index)
            x = round(x2d(k,1));
            y = round(x2d(k,2));
            ind = models{cls_index}(cad_index).ind(index(k),:);
            if x > pad_size && x <= size(mask,2)-pad_size && y > pad_size && y <= size(mask,1)-pad_size
                if mask(y,x) > 0 && mask(y,x) ~= j % occluded by other objects
                    occluder = find(index_object == mask(y,x));
                    if occluder > occludee
                        visibility_grid(ind(1), ind(2), ind(3)) = 2;
                    end
                end
            else
                visibility_grid(ind(1), ind(2), ind(3)) = 3;  % truncated
            end
        end
        objects(j).grid_origin = visibility_grid;        
    end
    record.objects = objects;
    
    % flip the mask
    mask_flip = mask(:,end:-1:1);
    record.mask_flip = mask_flip;
    
    % add flipped objects
    objects_flip = objects;
    for j = 1:num
        objects_flip(j).is_flip = 1;
        % flip bbox
        oldx1 = objects(j).bbox(1);
        oldx2 = objects(j).bbox(3);        
        objects_flip(j).bbox(1) = w - oldx2 + 1;
        objects_flip(j).bbox(3) = w - oldx1 + 1;        
        
        if isempty(objects(j).grid) == 1
            continue;
        end        
        
        % flip viewpoint
        azimuth = objects(j).viewpoint.azimuth;
        azimuth = 360 - azimuth;
        if azimuth >= 360
            azimuth = 360 - azimuth;
        end
        objects_flip(j).azimuth = azimuth;
        objects_flip(j).viewpoint.azimuth = azimuth;
        objects_flip(j).viewpoint.px = w - objects(j).viewpoint.px + 1;
        objects_flip(j).viewpoint.theta = -1 * objects(j).viewpoint.theta;
        elevation = objects(j).elevation;

        % flip pattern
        pattern = objects(j).pattern;
        objects_flip(j).pattern = pattern(:,end:-1:1);

        % 3D occlusion mask from the mean shape
        cls_index = strcmp(objects(j).class, classes) == 1;
        cad_index = objects(j).cad_index;  
        [visibility_grid, visibility_ind] = check_visibility(models_mean{cls_index}, azimuth, elevation);

        % check the occlusion status of visible voxels
        index = find(visibility_ind == 1);
        x3d = models_mean{cls_index}.x3d(index,:) * rescales{cls_index}(cad_index);
        x2d = project_3d(x3d, objects_flip(j));
        x2d = x2d + pad_size;
        occludee = find(index_object == j);
        for k = 1:numel(index)
            x = round(x2d(k,1));
            y = round(x2d(k,2));
            ind = models_mean{cls_index}.ind(index(k),:);
            if x > pad_size && x <= size(mask_flip,2)-pad_size && y > pad_size && y <= size(mask_flip,1)-pad_size
                if mask_flip(y,x) > 0 && mask_flip(y,x) ~= j % occluded by other objects
                    occluder = find(index_object == mask_flip(y,x));
                    if occluder > occludee
                        visibility_grid(ind(1), ind(2), ind(3)) = 2;
                    end
                end
            else
                visibility_grid(ind(1), ind(2), ind(3)) = 3;  % truncated
            end
        end
        objects_flip(j).grid = visibility_grid;
        
        % 3D occlusion mask from the original shape
        [visibility_grid, visibility_ind] = check_visibility(models{cls_index}(cad_index), azimuth, elevation);

        % check the occlusion status of visible voxels
        index = find(visibility_ind == 1);
        x3d = models{cls_index}(cad_index).x3d(index,:) * rescales{cls_index}(cad_index);
        x2d = project_3d(x3d, objects_flip(j));
        x2d = x2d + pad_size;
        occludee = find(index_object == j);
        for k = 1:numel(index)
            x = round(x2d(k,1));
            y = round(x2d(k,2));
            ind = models{cls_index}(cad_index).ind(index(k),:);
            if x > pad_size && x <= size(mask_flip,2)-pad_size && y > pad_size && y <= size(mask_flip,1)-pad_size
                if mask_flip(y,x) > 0 && mask_flip(y,x) ~= j % occluded by other objects
                    occluder = find(index_object == mask_flip(y,x));
                    if occluder > occludee
                        visibility_grid(ind(1), ind(2), ind(3)) = 2;
                    end
                end
            else
                visibility_grid(ind(1), ind(2), ind(3)) = 3;  % truncated
            end
        end
        objects_flip(j).grid_origin = visibility_grid;         
    end

    % save annotation
    record.objects_flip = objects_flip;
    parsave(sprintf('Annotations/%s.mat', ids{i}), record);
end

matlabpool close;

function parsave(fname, record)
save(fname, 'record')