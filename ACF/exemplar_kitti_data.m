function [pos, neg] = exemplar_kitti_data(cls, data, cid, is_train, is_continue)

% Get training data from the KITTI dataset.

exemplar_globals;

filename = [cachedir cls '_train_' num2str(cid) '.mat'];
if is_continue == 1 && exist(filename, 'file') ~= 0
  load(filename);
else
  % positive examples from kitti
  root_dir = KITTIroot;
  data_set = 'training';

  % get sub-directories
  cam = 2; % 2 = left color camera
  image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);
  label_dir = fullfile(root_dir, [data_set '/label_' num2str(cam)]);
  
  % get number of images for this dataset
  index = find(data.idx == cid);
  num_train = numel(index);
  
  pos = [];
  numpos = 0;
  for i = 1:num_train
    fprintf('%s %d: parsing positives: %d/%d\n', cls, cid, i, num_train);
    ind = index(i);
    numpos = numpos+1;
    pos(numpos).im = fullfile(image_dir, data.imgname{ind});
    bbox = data.bbox(:,ind);
    pos(numpos).x1 = bbox(1);
    pos(numpos).y1 = bbox(2);
    pos(numpos).x2 = bbox(3);
    pos(numpos).y2 = bbox(4);
    pos(numpos).bbox = bbox';
    pos(numpos).flip = false;
    pos(numpos).trunc = data.truncation(ind);
  end

  % negative examples from kitti
  switch cls
      case 'car'
          str_pos = {'Car', 'Van', 'DontCare'};
      otherwise
          fprintf('undefined classes for negatives\n');
  end
  
  filename = fullfile(SLMroot, 'ACF/kitti_ids.mat');
  object = load(filename);
  if is_train
      ids = object.ids_train;
  else
      ids = [object.ids_train object.ids_val];
  end
  neg = [];
  numneg = 0;
  for i = 1:length(ids);
    fprintf('%s %d: parsing negatives: %d/%d\n', cls, cid, i, length(ids));
    numneg = numneg+1;
    neg(numneg).im = sprintf('%s/%06d.png', image_dir, ids(i));
    neg(numneg).flip = false;
    
    objects = readLabels(label_dir, ids(i));
    n = numel(objects);
    bbox = [];
    trunc = [];
    count = 0;
    for j = 1:n
        if sum(strcmp(objects(j).type, str_pos)) > 0
            count = count + 1;
            bbox(count,:) = [objects(j).x1 objects(j).y1 objects(j).x2 objects(j).y2];
            trunc(count) = objects(j).truncation;
        end
    end
    neg(numneg).bbox = bbox;
    trunc(trunc == -1) = 0;
    neg(numneg).trunc = trunc;
  end
  
  save([cachedir cls '_train_' num2str(cid)], 'pos', 'neg');
end  