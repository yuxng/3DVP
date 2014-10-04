% show kitti annotations
function show_annotations

opt = globals();
pascal_init;
pad_size = 100;

% load PASCAL3D+ cad models
fprintf('load CAD models from file\n');
object = load('cads.mat');
cads = object.cads;
classes = cads.classes;
rescales = cads.rescales;
models = cads.models;

ids = textread(sprintf(VOCopts.imgsetpath, 'trainval'), '%s');

% get number of images for this dataset
nimages = length(ids);

% main loop
figure(1);
cmap = colormap(jet);
for img_idx = 1:nimages
    disp(img_idx);
  % show image
  subplot(1, 2, 1);
  filename = sprintf(VOCopts.imgpath, ids{img_idx});
  I = imread(filename);
  [h, w, ~] = size(I);
  mask = ones(h, w, 3);
  mask = padarray(mask, [pad_size pad_size 0]);
  imshow(I);
  hold on;
  
  % load annotations
  filename = sprintf('Annotations/%s.mat', ids{img_idx});
  object = load(filename);
  objects = object.record.objects;
  
  % sort objects from large distance to small distance
  index = sort_objects(objects);
 
  % for all annotated objects do
  for i = 1:numel(index)
    obj_idx = index(i);
    % plot 2D bounding box
    object = objects(obj_idx);
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
        
        index_color = 1 + floor((i-1) * size(cmap,1) / numel(index));
        patch('vertices', x2d, 'faces', face, ...
            'FaceColor', cmap(index_color,:), 'FaceAlpha', 0.2, 'EdgeColor', 'none');
        
        x2d = x2d + pad_size;
        vertices = [x2d(face(:,1),2) x2d(face(:,1),1) ...
                    x2d(face(:,2),2) x2d(face(:,2),1) ...
                    x2d(face(:,3),2) x2d(face(:,3),1)];

        BW = mesh_test(vertices, h+2*pad_size, w+2*pad_size);
        
        for j = 1:3
            tmp = mask(:,:,j);
            tmp(BW) = cmap(index_color,j);
            mask(:,:,j) = tmp;
        end
    end
    
  end
  hold off;
  
  subplot(1,2,2);
  mask = mask(pad_size+1:h+pad_size, pad_size+1:w+pad_size,:);
  imshow(uint8(255*mask));
  axis off;  
  axis equal;
  
  pause;
end