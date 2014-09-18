function [H_wo, X_wo, margin] = find_MVC(W_s, W_a, centers, id)

% load groundtruth feature
filename = fullfile('FEAT_TRUE_TRAINVAL', sprintf('%04d.mat', id));
object = load(filename);
Feat_true = object.Feat_true;

% load detections
filename = fullfile('CACHED_DATA_TRAINVAL', sprintf('%04d.mat', id));
object = load(filename);
Detections = object.Detections;
Scores = object.Scores;

% load loss
filename = fullfile('LOSS_TRAINVAL', sprintf('%04d.mat', id));
object = load(filename);
loss = object.loss;
nDet = size(Detections, 1);

% Initial energy is just the weighted local scores 
E = zeros(nDet, 1);
for clsID = 1:numel(centers)
    cls_dets = find(Detections(:, 5) == centers(clsID));
    if isempty(cls_dets) == 0
        E(cls_dets) = W_a(2*clsID - 1) .* Scores(cls_dets) + W_a(2*clsID);
    end
end

Pos = E + loss(:, 1);
Neg = loss(:, 2);

[E_mex, H_wo] = maximize(double(Detections(1:nDet, :)), Pos, Neg, W_s);
inds = find(H_wo == 1);
[PSI_wo_mex, PHI_wo_mex] = computeFeature(double(Detections(inds, :)), double(Scores(inds)));
Feature_wo = [PSI_wo_mex; PHI_wo_mex];

X_wo = Feat_true -  Feature_wo;
bg = H_wo ==0;
I = H_wo == 1;
margin = sum(loss(I,1)) + sum(loss(bg, 2));
% mvc_score = [W_s; W_a]'*[PSI_wo_mex; PHI_wo_mex] + margin;