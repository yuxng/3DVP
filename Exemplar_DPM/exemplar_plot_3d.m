function exemplar_plot_3d(img_idx, objects)

opt = my_globals();

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
calib_dir = fullfile(root_dir,[data_set '/calib']);

figure(1);
% show image
subplot(2, 1, 1);
I = imread(sprintf('%s/%06d.png',image_dir, img_idx));
imshow(I);
hold on;

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

Vgt = [];
Fgt = [];

% for all annotated objects do
for i = 1:numel(objects)
    % plot 2D bounding box
    object = objects(i);
    draw_2d_box(object);

    if strcmp(object.type, 'Car') == 1
        cad_index = find_closest_cad(cads, object);
        x3d = compute_3d_points(cads(cad_index).vertices, object);
        face = cads(cad_index).faces;

        tmp = face + size(Vgt,2);
        Fgt = [Fgt; tmp];        
        Vgt = [Vgt x3d];
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