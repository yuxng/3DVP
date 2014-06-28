% show kitti annotations
function show_annotations

opt = globals();
pad_size = 100;

% load PASCAL3D+ cad models
cls = 'car';
filename = sprintf(opt.path_cad, cls);
object = load(filename);
cads = object.(cls);
cads([7, 8, 10]) = [];

root_dir = opt.path_kitti_root;
data_set = 'training';

% get sub-directories
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir,[data_set '/image_' num2str(cam)]);
label_dir = fullfile(root_dir,[data_set '/label_' num2str(cam)]);
calib_dir = fullfile(root_dir,[data_set '/calib']);

% get number of images for this dataset
nimages = length(dir(fullfile(image_dir, '*.png')));

% main loop
figure(1);
cmap = colormap(jet);
for img_idx = 1:nimages-1
  % show image
  subplot(2, 1, 1);
  I = imread(sprintf('%s/%06d.png',image_dir, img_idx));
  [h, w, ~] = size(I);
  mask = ones(h, w, 3);
  mask = padarray(mask, [pad_size pad_size 0]);
  imshow(I);
  hold on;

  % load projection matrix
  P = readCalibration(calib_dir, img_idx, cam);
  
  % load labels
  objects = readLabels(label_dir,img_idx);
  
  % sort objects from large distance to small distance
  index = sort_objects(objects);
 
  % for all annotated objects do
  for i = 1:numel(index)
    obj_idx = index(i);
    % plot 2D bounding box
    object = objects(obj_idx);
    draw_2d_box(object);
    
    if strcmp(object.type, 'Car') == 1
        cad_index = find_closest_cad(cads, object);
        x3d = compute_3d_points(cads(cad_index).vertices, object);
        x2d = projectToImage(x3d, P);
        face = cads(cad_index).faces;
        index_color = 1 + floor((i-1) * size(cmap,1) / numel(index));
        
        x2d = x2d';
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
  
  subplot(2,1,2);
  mask = mask(pad_size+1:h+pad_size, pad_size+1:w+pad_size,:);
  imshow(uint8(255*mask));
  axis off;  
  axis equal;
  pause;
end