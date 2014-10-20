function [pos, neg, spos] = exemplar_pascal_data_joint(cls, name, data, centers, is_train, is_continue)

% Get training data from the PASCAL3D+ dataset.

globals; 
pascal_init;

filename = [cachedir cls '_' name '_train.mat'];
if is_continue == 1 && exist(filename, 'file') ~= 0
  load(filename);
else
  % positive examples from pascal3d
  index = find(data.idx ~= -1);
  num_train = numel(index);
  
  pos = [];
  cids = [];
  numpos = 0;
  for i = 1:num_train
    fprintf('%s %s : parsing positives: %d/%d\n', cls, name, i, num_train);
    ind = index(i);
    bbox = data.bbox(:,ind);
    id = data.id{ind};
    w = bbox(3) - bbox(1);
    h = bbox(4) - bbox(2);
    if w > 1 && h > 1 
        numpos = numpos+1;
        pos(numpos).im = sprintf(VOCopts.imgpath, id);
        pos(numpos).x1 = bbox(1);
        pos(numpos).y1 = bbox(2);
        pos(numpos).x2 = bbox(3);
        pos(numpos).y2 = bbox(4);
        pos(numpos).bbox = bbox';
        pos(numpos).flip = data.is_flip(ind) == 1;
        pos(numpos).trunc = 0;
        cids(numpos) = data.idx(ind);
    end
  end
  
  % split positive examples
  num = numel(centers);
  spos = cell(1,num);
  for i = 1:num
      index = cids == centers(i);
      spos{i} = pos(index);
  end

  % negative examples from pascal
  if is_train
      ids = textread(sprintf(VOCopts.imgsetpath, 'train'), '%s');
  else
      ids = textread(sprintf(VOCopts.imgsetpath, 'trainval'), '%s');
  end
  neg = [];
  numneg = 0;
  for i = 1:length(ids);
    fprintf('%s %s : parsing negatives: %d/%d\n', cls, name, i, length(ids));
    numneg = numneg+1;
    neg(numneg).im = sprintf(VOCopts.imgpath, ids{i});
    neg(numneg).flip = false;
    
    rec = PASreadrecord(sprintf(VOCopts.annopath, ids{i}));
    objects = rec.objects;
    n = numel(objects);
    bbox = [];
    count = 0;
    for j = 1:n
        if strcmp(objects(j).class, cls) == 1
            count = count + 1;
            bbox(count,:) = objects(j).bbox;
        end
    end
    neg(numneg).bbox = bbox;
  end
  
  save(filename, 'pos', 'neg', 'spos');
end  