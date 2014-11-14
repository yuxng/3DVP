% show kitti annotations
function show_3D_annotations

opt = globals();
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

for img_idx = 1138:nimages-1
  % show image
  I = imread(sprintf('%s/%06d.png',image_dir, img_idx));
  imshow(I);
  hold on;

  % load projection matrix
  P = readCalibration(calib_dir, img_idx, cam);
  
  % load labels
  objects = readLabels(label_dir, img_idx);
 
  % for all annotated objects do
  for i = 1:numel(objects)
    obj_idx = i;
    object = objects(obj_idx);
    
    if strcmp(object.type, 'Car') == 1
        % plot 3D bounding box
        [corners, face_idx] = computeBox3D(object, P);
        orientation = computeOrientation3D(object, P);
        draw_box_3D(object, corners, face_idx, orientation);
    end
    
  end
  hold off;
  
  pause;
end


function draw_box_3D(object, corners, face_idx, orientation)

% set styles for occlusion and truncation
occ_col    = {'g','y','r','w'};
trun_style = {'-','--'};
trc        = double(object.truncation>0.1)+1;

% draw projected 3D bounding boxes
if ~isempty(corners)
    for f=1:4
      line([corners(1,face_idx(f,:)),corners(1,face_idx(f,1))]+1,...
           [corners(2,face_idx(f,:)),corners(2,face_idx(f,1))]+1,...
           'color',occ_col{object.occlusion+1},...
           'LineWidth',3,'LineStyle',trun_style{trc});
      line([corners(1,face_idx(f,:)),corners(1,face_idx(f,1))]+1,...
           [corners(2,face_idx(f,:)),corners(2,face_idx(f,1))]+1,...
           'color','b','LineWidth',1);
    end
end

% draw orientation vector
if ~isempty(orientation)
line([orientation(1,:),orientation(1,:)]+1,...
     [orientation(2,:),orientation(2,:)]+1,...
    'color','w','LineWidth',4);
line([orientation(1,:),orientation(1,:)]+1,...
     [orientation(2,:),orientation(2,:)]+1,...
     'color','k','LineWidth',2);
end