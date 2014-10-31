function pick = nms_pairwise(boxes, pairwise, overlap, fixed)
% pick = nms_pairwise(boxes, pairwise, overlap)
% Non-maximum suppression.
% Greedily select high-scoring detections and skip detections
% that are significantly covered by a previously selected detection.
if(nargin < 4)
    fixed = 0;
end

if isempty(boxes)
    pick = [];
else
    s = boxes(:,end);
    [~, I] = sort(s, 'descend');
    n = numel(I);
    pick = ones(n, 1);
    for i = fixed+1:n
        ii = I(i);
        for j = 1:i-1
            jj = I(j);
            if pick(jj)
                if(pairwise(ii, jj) > overlap)
                pick(ii) = 0;
                break;
            end
        end
    end
end
pick = find(pick == 1);
end