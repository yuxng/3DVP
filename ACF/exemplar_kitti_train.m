function detector = exemplar_kitti_train(cls, data, cid)

[pos, neg] = exemplar_kitti_data(cls, data, cid);

% set up opts for training detector (see acfTrain)
opts = exemplar_acf_train();
modelDs = compute_model_size(pos);
opts.modelDs = modelDs;
opts.modelDsPad = round(1.2*modelDs);
opts.pos = pos;
opts.neg = neg;
opts.nWeak = [32 128 512 2048];
opts.pJitter = struct('flip',1);
opts.pBoost.pTree.fracFtrs = 1/16;
opts.pLoad = {'squarify', {3,.41}};
opts.name = sprintf('data/%s_%d', cls, cid);

% train detector (see acfTrain)
detector = exemplar_acf_train( opts );


function modelDs = compute_model_size(pos)

% pick mode of aspect ratios
h = [pos(:).y2]' - [pos(:).y1]' + 1;
w = [pos(:).x2]' - [pos(:).x1]' + 1;
xx = -2:.02:2;
filter = exp(-[-100:100].^2/400);
aspects = hist(log(h./w), xx);
aspects = convn(aspects, filter, 'same');
[~, I] = max(aspects);
aspect = exp(xx(I));

% pick 20 percentile area
areas = sort(h.*w);
area = areas(floor(length(areas) * 0.2));
area = max(min(area, 5000), 500);

% pick dimensions
w = sqrt(area/aspect);
h = w*aspect;
modelDs = round([h w]);