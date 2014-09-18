% PSI: pairwise feature
% PHI: unary feature
function [PSI, PHI] = compute_feature(I, Detections, Scores, Patterns, centers)

% pairwise feature
PSI = zeros(2, 1);
width = size(I, 2);
height = size(I, 1);

% compute pattern matching scores
num = size(Detections, 1);
for i = 1:num
    di = Detections(i,2) + Detections(i,4);   
    pi = uint8(zeros(height, width));
    pattern = Patterns{i};
    h = size(pattern, 1);
    w = size(pattern, 2);
    x = max(1, floor(Detections(i, 1)));
    y = max(1, floor(Detections(i, 2)));
    index_y = y:min(y+h-1, height);
    index_x = x:min(x+w-1, width);
    pi(index_y, index_x) = pattern(1:numel(index_y), 1:numel(index_x)); 
    
    for j = i+1:num
        dj = Detections(j,2) + Detections(j,4);
        pj = uint8(zeros(height, width));
        pattern = Patterns{j};
        h = size(pattern, 1);
        w = size(pattern, 2);
        x = max(1, floor(Detections(j, 1)));
        y = max(1, floor(Detections(j, 2)));
        index_y = y:min(y+h-1, height);
        index_x = x:min(x+w-1, width);
        pj(index_y, index_x) = pattern(1:numel(index_y), 1:numel(index_x));     
        
        index = pi > 0 & pj > 0;
        overlap = sum(sum(index));
        
        if di < dj  % object i is occluded
            ri = overlap / sum(sum(pi > 0));
            if ri < 0.1
                s = 1;
            else
                s = sum(pi(index) == 2) / overlap;
            end
        else  % object j is occluded
            rj = overlap / sum(sum(pj > 0));            
            if rj < 0.1
                s = 1;
            else
                s = sum(pj(index) == 2) / overlap;
            end
        end
        
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