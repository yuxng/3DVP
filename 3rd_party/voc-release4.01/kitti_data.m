function [pos, neg] = kitti_data(cls, flippedpos, year)

% [pos, neg] = kitti_data(cls)
% Get training data from the KITTI dataset.

setVOCyear = year;
globals; 
pascal_init;

if nargin < 2
  flippedpos = false;
end

try
  load([cachedir cls '_train_' year]);
catch
  % positive examples from kitti
  root_dir = KITTIroot;
  data_set = 'training';

  % get sub-directories
  cam = 2; % 2 = left color camera
  image_dir = fullfile(root_dir,[data_set '/image_' num2str(cam)]);
  label_dir = fullfile(root_dir,[data_set '/label_' num2str(cam)]);  
  
  % get number of images for this dataset
  nimages = length(dir(fullfile(image_dir, '*.png')));
  num_train = floor(nimages/2);
  
  pos = [];
  numpos = 0;
  for img_idx = 0:num_train-1
    fprintf('%s: parsing positives: %d/%d\n', cls, img_idx+1, num_train);
    % load labels
    objects = readLabels(label_dir, img_idx);
    clsinds = strmatch(cls, lower({objects(:).type}), 'exact');
    for j = clsinds(:)'
      numpos = numpos+1;
      pos(numpos).im = sprintf('%s/%06d.png',image_dir, img_idx);
      info = imfinfo(pos(numpos).im);
      bbox = [objects(j).x1 objects(j).y1 objects(j).x2 objects(j).y2];
      pos(numpos).x1 = bbox(1);
      pos(numpos).y1 = bbox(2);
      pos(numpos).x2 = bbox(3);
      pos(numpos).y2 = bbox(4);
      pos(numpos).flip = false;
      pos(numpos).trunc = objects(j).truncation > 0;
      if flippedpos
        oldx1 = bbox(1);
        oldx2 = bbox(3);
        bbox(1) = info.Width - oldx2 + 1;
        bbox(3) = info.Width - oldx1 + 1;
        numpos = numpos+1;
        pos(numpos).im = sprintf('%s/%06d.png',image_dir, img_idx);
        pos(numpos).x1 = bbox(1);
        pos(numpos).y1 = bbox(2);
        pos(numpos).x2 = bbox(3);
        pos(numpos).y2 = bbox(4);
        pos(numpos).flip = true;
        pos(numpos).trunc = objects(j).truncation > 0;
      end
    end
  end

  % negative examples from train (this seems enough!)
  ids = textread(sprintf(VOCopts.imgsetpath, 'train'), '%s');
  neg = [];
  numneg = 0;
  for i = 1:length(ids);
    fprintf('%s: parsing negatives: %d/%d\n', cls, i, length(ids));
    rec = PASreadrecord(sprintf(VOCopts.annopath, ids{i}));
    clsinds = strmatch(cls, {rec.objects(:).class}, 'exact');
    if length(clsinds) == 0
      numneg = numneg+1;
      neg(numneg).im = [VOCopts.datadir rec.imgname];
      neg(numneg).flip = false;
    end
  end
  
  save([cachedir cls '_train_' year], 'pos', 'neg');
end  