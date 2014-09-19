% PSI: pairwise feature
% PHI: unary feature
function [PSI, PHI] = compute_feature(Detections, Scores, Matching, centers)

% pairwise feature
PSI = zeros(2, 1);

% compute pattern matching scores
num = size(Detections, 1);
for i = 1:num
    for j = i+1:num
        s = Matching(i,j);
        PSI(1) = PSI(1) + s;
        PSI(2) = PSI(2) + 1;
    end
end

% unary feature
n = numel(centers);
PHI = zeros(2*n, 1);

for i = 1:num
    cid = Detections(i, 5);
    index = find(centers == cid);
    PHI(2*index - 1) = PHI(2*index - 1) + Scores(i);
    PHI(2*index) = PHI(2*index) + 1;
end