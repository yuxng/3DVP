function show_backprojection

opt = globals();

% load the mean CAD model
cls = 'car';
filename = sprintf('%s/%s_mean.mat', opt.path_slm_geometry, cls);
object = load(filename);
cad = object.(cls);
vertices = cad.x3d;

% load PASCAL3D+ cad models
filename = sprintf(opt.path_cad, cls);
object = load(filename);
cads = object.(cls);
cads([7, 8, 10]) = [];

% load data
object = load('data.mat');
data = object.data;

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
  disp(img_idx);
  % show image
  subplot(2, 1, 1);
  I = imread(sprintf('%s/%06d.png',image_dir, img_idx));
  imshow(I);
  hold on;
  
  % projection matrix
  P = readCalibration(calib_dir, img_idx, cam);

  % load the velo_to_cam matrix
  R0_rect = readCalibration(calib_dir, img_idx, 4);
  tmp = R0_rect';
  tmp = tmp(1:9);
  tmp = reshape(tmp, 3, 3);
  tmp = tmp';
  Pv2c = readCalibration(calib_dir, img_idx, 5);
  Pv2c = tmp * Pv2c;
  Pv2c = [Pv2c; 0 0 0 1];
  
  % camera location in world
  C = Pv2c\[0; 0; 0; 1];
  C(4) = [];  
  
  % load labels
  objects = readLabels(label_dir,img_idx);
  
  % sort objects from large distance to small distance
  index = sort_objects(objects);
  
  Vgt = [];
  Fgt = [];
  T = [];
 
  % for all annotated objects do
  for i = 1:numel(index)
    obj_idx = index(i);
    % plot 2D bounding box
    object = objects(obj_idx);
    draw_2d_box(object);
    
    if strcmp(object.type, 'Car') == 1
        cad_index = find_closest_cad(cads, object);
        x3d = compute_3d_points(cads(cad_index).vertices, object);
        if object.truncation == 0
            width = object.x2 - object.x1;
            height = object.y2 - object.y1;
        else
            x2d = projectToImage(x3d, P);
            width = max(x2d(1,:)) - min(x2d(1,:));
            height = max(x2d(2,:)) - min(x2d(2,:));
        end
        face = cads(cad_index).faces;
        
        tmp = face + size(Vgt,2);
        Fgt = [Fgt; tmp];        
        Vgt = [Vgt x3d];
        
        if object.truncation == 0
            % get the bounding box center
            c = [(object.x1 + object.x2)/2; (object.y1+object.y2)/2; 1];
        else
            % use the object center instead of bounding box center
            x3d = compute_3d_points_noscaling([0 0 0], object);
            x2d = projectToImage(x3d, P);
            c = [x2d; 1];
        end
        % backprojection
        X = pinv(P) * c;
        X = X ./ X(4);
        if X(3) < 0
            X = -1 * X;
        end        
        % transform to velodyne space
        X = Pv2c\X;
        X(4) = [];
        % compute the ray
        X = X - C;
        % normalization
        X = X ./ norm(X);     
        
        % optimization to search for 3D bounding box
        % initialization
        x = zeros(1,5);
        x(1) = mean(data.l);
        x(2) = mean(data.h);
        x(3) = mean(data.w);
        x(4) = object.ry;
        x(5) = mean(data.distance);
        % compute lower bound and upper bound
        lb = [min(data.l) min(data.h) min(data.w) x(4)-15*pi/180 min(data.distance)];
        ub = [max(data.l) max(data.h) max(data.w) x(4)+15*pi/180 max(data.distance)];
        % optimize
        options = optimset('Algorithm', 'interior-point');
        x = fmincon(@(x)compute_error_distance(x, vertices, C, X, P, Pv2c, width, height),...
            x, [], [], [], [], lb, ub, [], options);
        disp(x);
        disp([object.l object.h object.w object.ry]);
        T = [T C+x(5).*X];
    end
  end
  hold off;
  
  subplot(2,1,2); 
  if isempty(Vgt) == 0
      Vgt = Pv2c\[Vgt; ones(1,size(Vgt,2))];
      trimesh(Fgt, Vgt(1,:), Vgt(2,:), Vgt(3,:), 'EdgeColor', 'b');
      hold on;
      axis equal;
      xlabel('x');
      ylabel('y');
      zlabel('z');
      
      for i = 1:size(T,2)
        plot3([C(1) T(1,i)], [C(2) T(2,i)], [C(3) T(3,i)], 'g');
      end

      % draw the camera
      draw_camera(C);
      
      % draw the ground plane
      h = 1.73;
      s = max(max(abs(Vgt(1:2,:))));
      c = [mean(Vgt(1:2,:), 2); 0]';
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
%   break;
  pause;
end

% compute the projection error between 3D bbox and 2D bbox
function error = compute_error_distance(x, vertices, C, X, P, Pv2c, bw, bh)

% 3D bounding box dimensions
object.l = x(1);
object.h = x(2);
object.w = x(3);
object.ry = x(4);

% compute the translate of the 3D bounding box
t = C + x(5) .* X;
t = Pv2c*[t; 1];
t(4) = [];
object.t = t;

% compute 3D points
x3d = compute_3d_points(vertices, object);

% project the 3D bounding box into the image plane
x2d = projectToImage(x3d, P);

% compute bounding box width and height
width = max(x2d(1,:)) - min(x2d(1,:));
height = max(x2d(2,:)) - min(x2d(2,:));

% compute error
error = (width - bw)^2 + (height - bh)^2;