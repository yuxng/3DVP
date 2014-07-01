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

% train root filters using warped positives & random negatives
try
  load([cachedir cls '_' num2str(cid) '_lrsplit1']);
catch
  initrand();
  model = initmodel(cls, pos, note, 'N');
  model = train(cls, model, pos, neg, 1, 1, 1, 1, ...
                      cachesize, true, 0.7, false, 'lrsplit1');
  save([cachedir cls '_' num2str(cid) '_lrsplit1'], 'models');
end

% train root left vs. right facing root filters using latent detections
% and hard negatives
try
  load([cachedir cls '_lrsplit2']);
catch
  initrand();
  for i = 1:n
    models{i} = lrmodel(models{i});
    models{i} = train(cls, models{i}, spos{i}, neg(1:maxneg), 0, 0, 4, 3, ...
                      cachesize, true, 0.7, false, ['lrsplit2_' num2str(i)]);
  end
  save([cachedir cls '_lrsplit2'], 'models');
end

% merge models and train using latent detections & hard negatives
try 
  load([cachedir cls '_mix']);
catch
  initrand();
  model = mergemodels(models);
  model = train(cls, model, pos, neg(1:maxneg), 0, 0, 1, 5, ...
                cachesize, true, 0.7, false, 'mix');
  save([cachedir cls '_mix'], 'model');
end

% add parts and update models using latent detections & hard negatives.
try 
  load([cachedir cls '_parts']);
catch
  initrand();
  for i = 1:2:2*n
    model = model_addparts(model, model.start, i, i, 8, [6 6]);
  end
  model = train(cls, model, pos, neg(1:maxneg), 0, 0, 8, 10, ...
                cachesize, true, 0.7, false, 'parts_1');
  model = train(cls, model, pos, neg, 0, 0, 1, 5, ...
                cachesize, true, 0.7, true, 'parts_2');
  save([cachedir cls '_parts'], 'model');
end

save([cachedir cls '_final'], 'model');