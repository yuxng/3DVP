function model = kitti_train(cls, data, cid, note)

% model = kitti_train(cls, data, cid, note)
% Train a model with 1 component using the KITTI dataset.
% note allows you to save a note with the trained model
% example: note = 'testing FRHOG (FRobnicated HOG)

% At every "checkpoint" in the training process we reset the 
% RNG's seed to a fixed value so that experimental results are 
% reproducible.
initrand();

if nargin < 4
  note = '';
end

globals; 
[pos, neg] = kitti_data(cls, data, cid, false);

cachesize = 24000;
maxneg = 200;

% train root filter using warped positives & random negatives
try
  load([cachedir cls '_' num2str(cid) '_wrap']);
catch
  initrand();
  name = [cls '_' num2str(cid)];
  model = initmodel(name, pos, note, 'N');
  model.symmetric = 0;
  model = train(name, model, pos, neg, 1, 1, 1, 1, ...
                      cachesize, true, 0.7, false, 'wrap');
  save([cachedir cls '_' num2str(cid) '_wrap'], 'model');
end

% train root filter using latent detections & hard negatives
try 
  load([cachedir cls '_' num2str(cid) '_latent']);
catch
  initrand();
  name = [cls '_' num2str(cid)];
  model = train(name, model, pos, neg(1:maxneg), 0, 0, 1, 5, ...
                cachesize, true, 0.7, false, 'latent');
  save([cachedir cls '_' num2str(cid) '_latent'], 'model');
end

% add parts and update model using latent detections & hard negatives.
try 
  load([cachedir cls '_' num2str(cid) '_parts']);
catch
  initrand();
  if min(model.filters(1).size) > 3
    model = model_addparts(model, model.start, 1, 1, 8, [6 6]);
  else
    model = model_addparts(model, model.start, 1, 1, 8, [3 3]);
  end
  name = [cls '_' num2str(cid)];
  model = train(name, model, pos, neg(1:maxneg), 0, 0, 8, 10, ...
                cachesize, true, 0.7, false, 'parts_1');
  model = train(name, model, pos, neg, 0, 0, 1, 5, ...
                cachesize, true, 0.7, true, 'parts_2');
  save([cachedir cls '_' num2str(cid) '_parts'], 'model');
end

save([cachedir cls '_' num2str(cid) '_final'], 'model');