% create occlusion masks for occluded objects
function create_occlusion_masks(issave)

if nargin < 1
    issave = 0;
end

opt = globals();
pad_size = 1000;
nplot = 5;
mplot = 6;
vnum = 8;

% load PASCAL3D+ cad models
cls = 'car';
filename = sprintf('../Geometry/%s.mat', cls);
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
if issave == 0
    figure(1);
    cmap = colormap(jet);
end
count = 0;
for img_idx = 0:nimages-1
  fprintf('image %06d\n', img_idx);
  % show image
  I = imread(sprintf('%s/%06d.png',image_dir, img_idx));
  [h, w, ~] = size(I);
  mask = zeros(h, w);
  mask = padarray(mask, [pad_size pad_size]);
  
  if issave == 0
    subplot(nplot, mplot, 1:mplot/2);
    imshow(I);
    hold on;
  end

  % load projection matrix
  P = readCalibration(calib_dir, img_idx, cam);
  
  % load labels
  objects = readLabels(label_dir,img_idx);
  
  % sort objects from large distance to small distance
  index = sort_objects(objects);
 
  % for all annotated objects do
  num = numel(index);
  BWs = cell(num, 1);
  for i = 1:num
    obj_idx = index(i);
    % plot 2D bounding box
    object = objects(obj_idx);
    
    if issave == 0
        draw_2d_box(object);
    end
    
    if strcmp(object.type, 'Car') == 1
        cad_index = find_closest_cad(cads, object);
        x3d = compute_3d_points(cads(cad_index).vertices, object);
        x2d = projectToImage(x3d, P);
        face = cads(cad_index).faces;
        x2d = x2d';
        
        flag = min(x2d(:,1)) < 0 & max(x2d(:,1)) > w;
        if flag == 1
            continue;
        end
        
%         if issave == 0 && flag == 0
%             index_color = 1 + floor((i-1) * size(cmap,1) / numel(index));
%             patch('vertices', x2d, 'faces', face, ...
%                 'FaceColor', cmap(index_color,:), 'FaceAlpha', 0.2, 'EdgeColor', 'none');
%         end
        
        x2d = x2d + pad_size;
        vertices = [x2d(face(:,1),2) x2d(face(:,1),1) ...
                    x2d(face(:,2),2) x2d(face(:,2),1) ...
                    x2d(face(:,3),2) x2d(face(:,3),1)];

        BWs{obj_idx} = mesh_test(vertices, h+2*pad_size, w+2*pad_size);
        
        mask(BWs{obj_idx}) = obj_idx;
    end
  end
  mask = mask(pad_size+1:h+pad_size, pad_size+1:w+pad_size);
  mask = padarray(mask, [pad_size pad_size]);
  
  if issave == 0
    hold off;
  end
  
  % create occlusion patterns
  for i = 1:num
      if isempty(BWs{i}) == 1
          if issave == 0
            subplot(nplot, mplot, i+mplot);
            title('');
            cla;
            axis off;
          end
          continue;
      end
      
      pattern = uint8(BWs{i});
      pattern = 2*pattern;  % occluded
      pattern(mask == i) = 1;  % visible
      [x, y] = find(pattern > 0);
      pattern = pattern(min(x):max(x), min(y):max(y));
      
      % compute occlusion percentage
      occ = numel(find(pattern == 2)) / numel(find(pattern > 0));
      
      im = 255*ones(size(pattern,1), size(pattern,2), 3);
      color = [0 255 0];
      for j = 1:3
        tmp = im(:,:,j);
        tmp(pattern == 1) = color(j);
        im(:,:,j) = tmp;
      end
      color = [255 0 0];
      for j = 1:3
        tmp = im(:,:,j);
        tmp(pattern == 2) = color(j);
        im(:,:,j) = tmp;
      end
      
      im = uint8(im);
      % save occlusion pattern images
      if issave == 1 && isempty(find(pattern == 2, 1)) == 0
          count = count + 1;
          
          % partition the azimuth
          alpha = objects(i).alpha*180/pi;
          if alpha < 0
              alpha = alpha + 360;
          end
          alpha = alpha - 90;
          if alpha < 0
              alpha = alpha + 360;
          end
          index_view = find_interval(alpha, vnum);
          
          filename = sprintf('../Results/occlusion_patterns/view%d/%06d_%02d.jpg', index_view, img_idx, i);
          scale = 100 / size(im,2);
          im = imresize(im, scale);
          imwrite(im, filename);
      end
      
      if issave == 0
          subplot(nplot, mplot, i+mplot);
          imshow(uint8(im));
          axis off;  
          axis equal;
          title(sprintf('object %d: occ=%.2f', i, occ));
      end
  end
      
  if issave == 0
      for i = num+1+mplot:nplot*mplot
          subplot(nplot, mplot, i);
          title('');
          cla;
          axis off;
      end
      
      subplot(nplot, mplot, mplot/2+1:mplot);
      mask = mask(pad_size+1:h+pad_size, pad_size+1:w+pad_size);
      imagesc(mask);
      axis off;  
      axis equal;
      pause;
  end
end