function [pos, neg, impos] = exemplar_kitti_data(cls, data, cid, flippedpos)

% Get training data from the KITTI dataset.

conf       = voc_config;
dataset_fg = conf.training.train_set_fg;
cachedir   = conf.paths.model_dir;

if nargin < 4
  flippedpos = false;
end

try
  load([cachedir cls '_' dataset_fg '_' num2str(cid)]);
catch
  % positive examples from kitti
  root_dir = conf.kitti.root;
  data_set = 'training';

  % get sub-directories
  cam = 2; % 2 = left color camera
  image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);
  label_dir = fullfile(root_dir, [data_set '/label_' num2str(cam)]);
  
  % get number of images for this dataset
  index = find(data.idx2 == cid);
  num_train = numel(index);
  
  pos      = [];
  impos    = [];
  numpos   = 0;
  numimpos = 0;
  dataid   = 0;  
  for i = 1:num_train
    fprintf('%s %d: parsing positives: %d/%d\n', cls, cid, i, num_train);
    ind = index(i);
    numpos = numpos+1;
    dataid = dataid + 1;
    
    pos(numpos).im = fullfile(image_dir, data.imgname{ind});
    info = imfinfo(pos(numpos).im);
    bbox = data.bbox(:,ind)';
    pos(numpos).x1 = bbox(1);
    pos(numpos).y1 = bbox(2);
    pos(numpos).x2 = bbox(3);
    pos(numpos).y2 = bbox(4);
    pos(numpos).boxes = bbox;
    pos(numpos).flip = false;
    pos(numpos).trunc = 0;
    pos(numpos).dataids = dataid;
    pos(numpos).sizes   = (bbox(3)-bbox(1)+1)*(bbox(4)-bbox(2)+1);
    
    if flippedpos
      oldx1 = bbox(1);
      oldx2 = bbox(3);
      bbox(1) = info.Width - oldx2 + 1;
      bbox(3) = info.Width - oldx1 + 1;
      numpos = numpos+1;
      dataid = dataid + 1;
      
      pos(numpos).im = fullfile(image_dir, data.imgname{ind});
      pos(numpos).x1 = bbox(1);
      pos(numpos).y1 = bbox(2);
      pos(numpos).x2 = bbox(3);
      pos(numpos).y2 = bbox(4);
      pos(numpos).boxes = bbox;
      pos(numpos).flip = true;
      pos(numpos).trunc = 0;
      pos(numpos).dataids = dataid;
      pos(numpos).sizes   = (bbox(3)-bbox(1)+1)*(bbox(4)-bbox(2)+1);      
    end
    
    % Create one entry per foreground image in the impos array
    numimpos                = numimpos + 1;
    impos(numimpos).im      = fullfile(image_dir, data.imgname{ind});
    impos(numimpos).boxes   = zeros(1, 4);
    impos(numimpos).dataids = zeros(1, 1);
    impos(numimpos).sizes   = zeros(1, 1);
    impos(numimpos).flip    = false;

    dataid = dataid + 1;
    bbox   = data.bbox(:,ind)';
    impos(numimpos).boxes = bbox;
    impos(numimpos).dataids = dataid;
    impos(numimpos).sizes   = (bbox(3)-bbox(1)+1)*(bbox(4)-bbox(2)+1);

    % Create flipped example
    if flippedpos
      numimpos                = numimpos + 1;
      impos(numimpos).im      = fullfile(image_dir, data.imgname{ind});
      impos(numimpos).boxes   = zeros(1, 4);
      impos(numimpos).dataids = zeros(1, 1);
      impos(numimpos).sizes   = zeros(1, 1);
      impos(numimpos).flip    = true;
      unflipped_boxes         = impos(numimpos-1).boxes;

      dataid  = dataid + 1;
      bbox    = unflipped_boxes;
      oldx1   = bbox(1);
      oldx2   = bbox(3);
      bbox(1) = info.Width - oldx2 + 1;
      bbox(3) = info.Width - oldx1 + 1;

      impos(numimpos).boxes = bbox;
      impos(numimpos).dataids = dataid;
      impos(numimpos).sizes   = (bbox(3)-bbox(1)+1)*(bbox(4)-bbox(2)+1);
    end
  end

  % negative examples from kitti
  switch cls
      case 'car'
          str_pos = {'Car', 'Van', 'DontCare'};
      otherwise
          fprintf('undefined classes for negatives\n');
  end
  object = load('kitti_ids.mat');
  ids = object.ids_train;
  neg = [];
  numneg = 0;
  for i = 1:length(ids);
    fprintf('%s %d: parsing negatives: %d/%d\n', cls, cid, i, length(ids));
    dataid = dataid + 1;
    numneg = numneg+1;
    neg(numneg).im = sprintf('%s/%06d.png', image_dir, ids(i));
    neg(numneg).flip = false;
    neg(numneg).dataid = dataid;
    
    objects = readLabels(label_dir, ids(i));
    n = numel(objects);
    bbox = [];
    count = 0;
    for j = 1:n
        if sum(strcmp(objects(j).type, str_pos)) > 0
            count = count + 1;
            bbox(count,:) = [objects(j).x1 objects(j).y1 objects(j).x2 objects(j).y2];
        end
    end
    neg(numneg).bbox = bbox;
  end
  
  save([cachedir cls '_' dataset_fg '_' num2str(cid)], 'pos', 'neg', 'impos');
end  