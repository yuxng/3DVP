function loss = compute_loss(boxes, gt, idx, overlap_threshold)
% loss = computeloss(Detections, GroundTruth)
% loss(i,1) is the loss associated with turning on  candidate i
% loss(i,2) is the loss associated with turning off candidate i

n  = size(boxes, 1);
x1 = boxes(:, 1);
y1 = boxes(:, 2);
x2 = boxes(:, 3);
y2 = boxes(:, 4);  
ba = (x2-x1+1) .* (y2-y1+1);

% Compute the maximum overlap of each box with each ground truth
lp = zeros(n, 1);
ln = zeros(n, 1);

% Iterate through ground truth, and update box with new best overlap    
for i = 1:size(gt,1),
    gx1 = gt(i,1);
    gy1 = gt(i,2);
    gx2 = gt(i,3);
    gy2 = gt(i,4);
    ga  = (gx2-gx1+1) .* (gy2-gy1+1);

    xx1 = max(x1, gx1);
    yy1 = max(y1, gy1);
    xx2 = min(x2, gx2);
    yy2 = min(y2, gy2);

    w = xx2-xx1+1;
    h = yy2-yy1+1;
    I = find(w > 0 & h > 0);
    int   = w(I).*h(I);
    ov    = zeros(n,1);
    ov(I) = int ./ (ba(I) + ga - int);
    
    % Assign true positive
    [v, j] = max(ov);
    ln(j) = max(ln(j), v);
    lp = max(lp, ov);
end

centers = unique(idx);
num = numel(centers);
loss = zeros(num, 2);

for i = 1:num
    index = idx == centers(i);
    loss(i,1) = double(max(lp(index)) < overlap_threshold);
    loss(i,2) = double(max(ln(index)) > overlap_threshold);
end

% loss(:,1) = double(lp < overlap_threshold);
% loss(:,2) = double(ln > overlap_threshold);