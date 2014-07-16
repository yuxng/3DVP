function show_backprojection

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
  subplot(2, 1, 1);
  I = imread(sprintf('%s/%06d.png',image_dir, img_idx));
  imshow(I);
  hold on;

  % load the velo_to_cam matrix
  P = readCalibration(calib_dir, img_idx, 5);
  P = [P; 0 0 0 1];
  
  % load labels
  objects = readLabels(label_dir,img_idx);
  
  % sort objects from large distance to small distance
  index = sort_objects(objects);
  
  V = [];
  F = [];
 
  % for all annotated objects do
  for i = 1:numel(index)
    obj_idx = index(i);
    % plot 2D bounding box
    object = objects(obj_idx);
    draw_2d_box(object);
    
    if strcmp(object.type, 'Car') == 1
        cad_index = find_closest_cad(cads, object);
        x3d = compute_3d_points(cads(cad_index).vertices, object);
        face = cads(cad_index).faces;
        
        tmp = face + size(V,2);
        F = [F; tmp];        
        
        V = [V x3d];
    end
    
  end
  hold off;
  
  subplot(2,1,2); 
  if isempty(V) == 0
      V = P\[V; ones(1,size(V,2))];
      trimesh(F, V(1,:), V(2,:), V(3,:), 'EdgeColor', 'b');
      hold on;
      axis equal;
      xlabel('x');
      ylabel('y');
      zlabel('z');

      % draw the camera
      C = P\[0; 0; 0; 1];
      C(4) = [];
      draw_camera(C);
      
      % draw the ground plane
      h = 1.73;
      s = max(max(abs(V(1:2,:))));
      c = [mean(V(1:2,:), 2); 0]';
      plane_vertex = zeros(4,3);
      plane_vertex(1,:) = c + [-s -s -h];
      plane_vertex(2,:) = c + [s -s -h];
      plane_vertex(3,:) = c + [s s -h];
      plane_vertex(4,:) = c + [-s s -h];
      patch('Faces', [1 2 3 4], 'Vertices', plane_vertex, 'FaceColor', [0.5 0.5 0.5], 'FaceAlpha', 0.5);

      axis tight;
      view(30, 10);
      hold off;
  end
  pause;
end