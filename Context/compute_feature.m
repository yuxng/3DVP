% PSI: pairwise feature
% PHI: unary feature
function [PSI, PHI] = compute_feature(Detections, Scores, Matching, centers, idx)

% find the detection clusters which are in the scene
cdets = unique(idx);
cdets(cdets == -1) = [];

% pairwise feature
PSI = zeros(2, 1);

% compute pattern matching scores
num = numel(cdets);
for i = 1:num
    index_i = idx == cdets(i);
    for j = i+1:num
        index_j = idx == cdets(j);
        s = max(max(Matching(index_i,index_j)));
        PSI(1) = PSI(1) + s;
        PSI(2) = PSI(2) + 1;
    end
end

% unary feature
n = numel(centers);
PHI = zeros(2*n, 1);

for i = 1:num
    cid = Detections(cdets(i), 5);
    index = find(centers == cid);
    PHI(2*index - 1) = PHI(2*index - 1) + Scores(cdets(i));
    PHI(2*index) = PHI(2*index) + 1;
end