function idx = nms_clustering(boxes, overlap, K)

% pick = nms(boxes, overlap) 
% Non-maximum suppression.
% Greedily select high-scoring detections and skip detections
% that are significantly covered by a previously selected detection.

if isempty(boxes)
    idx = [];
else
    x1 = boxes(:,1);
    y1 = boxes(:,2);
    x2 = boxes(:,3);
    y2 = boxes(:,4);
    s = boxes(:,end);
    area = (x2-x1+1) .* (y2-y1+1);

    [~, I] = sort(s, 'descend');
    n = numel(I);
    pick = ones(n, 1);
    idx = 1:n;
    for i = 1:n
        ii = I(i);
        for j = 1:i-1
            jj = I(j);
            if pick(jj)
                xx1 = max(x1(ii), x1(jj));
                yy1 = max(y1(ii), y1(jj));
                xx2 = min(x2(ii), x2(jj));
                yy2 = min(y2(ii), y2(jj));
                w = xx2-xx1+1;
                h = yy2-yy1+1;
                if w > 0 && h > 0
                    % compute overlap 
                    o = w * h / (area(ii) + area(jj) - w*h);
                    o1 = w * h / area(ii);
                    o2 = w * h / area(jj);
                    if o > overlap || o1 > 0.95 || o2 > 0.95
                        pick(ii) = 0;
                        idx(ii) = idx(jj);
                        break;
                    end
                end
            end
        end
    end
end

% keep the top K detections for each cluster
centers = unique(idx);
num = numel(centers);
for i = 1:num
    index = find(idx == centers(i));
    if numel(index) <= K
        continue;
    end
    scores = boxes(index,6);
    [~, ind] = sort(scores, 'descend');
    index = index(ind);
    index(1:K) = [];
    idx(index) = -1;
end