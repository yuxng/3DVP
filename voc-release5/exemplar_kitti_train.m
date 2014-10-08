function model = exemplar_kitti_train(cls, data, cid, note, is_train, is_continue, is_pascal)
% Train a model.
%   model = pascal_train(cls, n, note)
%
%   The model will be a mixture of n star models, each of which
%   has 2 latent orientations.
%
% Arguments
%   cls           Object class to train and evaluate
%   n             Number of aspect ratio clusters to use
%                 (The final model has 2*n components)
%   note          Save a note in the model.note field that describes this model

% At every "checkpoint" in the training process we reset the 
% RNG's seed to a fixed value so that experimental results are 
% reproducible.
seed_rand();

% Default to no note
if nargin < 3
  note = '';
end

conf = voc_config();
cachedir = conf.paths.model_dir;

% Load the training data
if is_pascal
    [pos, neg, impos] = exemplar_pascal_data(cls, data, cid, false, is_train, is_continue);
else
    [pos, neg, impos] = exemplar_kitti_data(cls, data, cid, false, is_train, is_continue);
end

max_num_examples = conf.training.cache_example_limit;
num_fp           = conf.training.wlssvm_M;
fg_overlap       = conf.training.fg_overlap;

% Select a small, random subset of negative images
% All data mining iterations use this subset, except in a final
% round of data mining where the model is exposed to all negative
% images
num_neg   = length(neg);
neg_perm  = neg(randperm(num_neg));
neg_small = neg_perm(1:min(num_neg, conf.training.num_negatives_small));
neg_large = neg; % use all of the negative images

% Train one asymmetric root filter for each aspect ratio group
% using warped positives and random negatives
filename = [cachedir cls '_' num2str(cid) '_wrap.mat'];
if is_continue && exist(filename, 'file')
  load(filename);
else
  seed_rand();
  
  model = root_model(cls, pos, note);

  model = train(model, pos, neg_large, true, true, 1, 1, ...
                max_num_examples, fg_overlap, 0, false, [num2str(cid) '_wrap']);
  save(filename, 'model');
end

% train root filter using latent detections & hard negatives
filename = [cachedir cls '_' num2str(cid) '_latent.mat'];
if is_continue && exist(filename, 'file')
  load(filename);
else
  seed_rand();

  model = train(model, impos, neg_small, false, false, 1, 5, ...
                max_num_examples, fg_overlap, num_fp, false, [num2str(cid) '_latent']);
  save(filename, 'model');
end

% Train a mixture model with 2x resolution parts using latent positives
% and hard negatives
filename = [cachedir cls '_' num2str(cid) '_parts.mat'];
if is_continue && exist(filename, 'file')
  load(filename);
else
  seed_rand();
  
  % Add parts to each mixture component

  % Top-level rule for this component
  ruleind = 1;
  % Top-level rule for this component's mirror image
  partner = [];
  % Filter to interoplate parts from
  filterind = 1;
  
  if min(model.filters(1).size) >= 9
    model = model_add_parts(model, model.start, ruleind, ...
                            partner, filterind, 8, [6 6], 1);
  elseif min(model.filters(1).size) >= 5
    model = model_add_parts(model, model.start, ruleind, ...
                            partner, filterind, 8, [3 3], 1);
  else
    model = model_add_parts(model, model.start, ruleind, ...
                            partner, filterind, 4, [3 3], 1);      
  end
  
  % Enable learning location/scale prior
  bl = model.rules{model.start}(1).loc.blocklabel;
  model.blocks(bl).w(:)     = 0;
  model.blocks(bl).learn    = 1;
  model.blocks(bl).reg_mult = 1;

  % Train using several rounds of positive latent relabeling
  % and data mining on the small set of negative images
  model = train(model, impos, neg_small, false, false, 8, 10, ...
                max_num_examples, fg_overlap, num_fp, false, [num2str(cid) '_parts_1']);
  % Finish training by data mining on all of the negative images
  model = train(model, impos, neg_large, false, false, 1, 5, ...
                max_num_examples, fg_overlap, num_fp, true, [num2str(cid) '_parts_2']);
  save(filename, 'model');
end

if is_pascal
    resultdir = 'PASCAL/';
else
    resultdir = 'KITTI/';
end
if exist(resultdir, 'dir') == 0
    unix(['mkdir -p ' resultdir]);
end
save([resultdir cls '_' num2str(cid) '_final'], 'model');