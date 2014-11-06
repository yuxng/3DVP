function data = threshold_detections(data, th)

if(isempty(data.onedet))
    return;
end

idx = find(data.onedet(:, end) > th);

data.onedet = data.onedet(idx, :);
data.unaries = data.unaries(idx, :);
data.pairwise = data.pairwise(idx, :, :);
data.pairwise = data.pairwise(:, idx, :);

end