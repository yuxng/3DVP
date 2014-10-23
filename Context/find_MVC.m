function [I, X_wo, margin] = find_MVC(W_s, W_a, centers, id)

% load groundtruth feature
filename = fullfile('FEAT_TRUE_TRAINVAL', sprintf('%06d.mat', id));
object = load(filename);
Feat_true = object.Feat_true;

% load detections
filename = fullfile('CACHED_DATA_TRAINVAL', sprintf('%06d.mat', id));
object = load(filename, 'Detections', 'Scores', 'Matching', 'Overlaps', 'Idx');
Detections = object.Detections;
Scores = object.Scores;
Matching = object.Matching;
Overlaps = object.Overlaps;
det = [Detections Scores];
Idx = nms_clustering(det, 0.6, 15); 
Matching(Overlaps < 0.1) = 1;

% load loss
filename = fullfile('LOSS_TRAINVAL', sprintf('%06d.mat', id));
object = load(filename);
loss = object.loss;

% cluster of detections
cdets = unique(Idx);
cdets(cdets == -1) = [];
num = numel(cdets);

% Initial energy is just the weighted local scores 
E = zeros(num, 1);
for i = 1:numel(centers)
    index = find(Detections(cdets, 5) == centers(i));
    if isempty(index) == 0
        E(index) = W_a(2*i - 1) .* Scores(cdets(index)) + W_a(2*i);
    end
end

Pos = E + loss(:, 1);
Neg = loss(:, 2);

[I, S] = maximize(Matching, Idx, Pos, Neg, W_s);

idx = Idx;
n = numel(cdets);
for i = 1:n
    index = idx == cdets(i);
    if I(i) == 0
        idx(index) = -1;
    end    
end

[PSI_wo, PHI_wo] = compute_feature(Detections, Scores, Matching, centers, idx);
Feature_wo = [PSI_wo; PHI_wo];

X_wo = Feat_true -  Feature_wo;
bg = I == 0;
fg = I == 1;
margin = sum(loss(fg,1)) + sum(loss(bg, 2));
% mvc_score = [W_s; W_a]'*[PSI_wo_mex; PHI_wo_mex] + margin;