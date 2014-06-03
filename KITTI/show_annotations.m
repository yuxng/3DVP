% show kitti annotations
function show_annotations

opt = globals();

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
for img_idx = 1:nimages-1
  % show image  
  I = imread(sprintf('%s/%06d.png',image_dir, img_idx));
  imshow(I);
  hold on;

  % load projection matrix
  P = readCalibration(calib_dir, img_idx, cam);
  
  % load labels
  objects = readLabels(label_dir,img_idx);
 
  % for all annotated objects do
  for obj_idx=1:numel(objects)
   
    % plot 2D bounding box
    object = objects(obj_idx);
    draw_2d_box(object);
    
    if strcmp(object.type, 'Car') == 1
        cad_index = find_closest_cad(cads, object);
        x3d = compute_3d_points(cads(cad_index), object);
        x2d = projectToImage(x3d, P);
        face = cads(cad_index).faces;
        
        patch('vertices', x2d', 'faces', face, ...
            'FaceColor', 'blue', 'FaceAlpha', 0.2, 'EdgeColor', 'none');
    end
    
    % plot 3D bounding box
%     [corners,face_idx] = computeBox3D(objects(obj_idx),P);
%     orientation = computeOrientation3D(objects(obj_idx),P);
%     drawBox3D(h, objects(obj_idx),corners,face_idx,orientation);
    
  end
  hold off;
  pause;
end