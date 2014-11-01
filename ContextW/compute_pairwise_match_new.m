function pairwise = compute_pairwise_match_new(dets, params)
% the function takes quite long time 
% we should mex it.

[imptns, dets_new] = get_image_patterns(dets, params.pattern);
pairwise = zeros(size(dets, 1), size(dets, 1), 2);
[pairwise(:,:,1), pairwise(:,:,2)] = compute_matching_scores(dets_new, imptns);

function [imptns, dets_new] = get_image_patterns(dets, patterns)

dets(:, 1:4) = round(dets(:, 1:4));
canvas = [min(dets(:, 1)), min(dets(:, 2)), max(dets(:, 3)), max(dets(:, 4))];

num = size(dets, 1);
height = canvas(4) - canvas(2) + 1;
width = canvas(3) - canvas(1) + 1;
imptns = uint8(zeros(height, width, num));
dets_new = zeros(num, 4);

for i = 1:num
    rawptn = uint8(zeros(height, width));
    
    h = dets(i, 4) - dets(i, 2) + 1;
    w = dets(i, 3) - dets(i, 1) + 1;
    
    ptn = imresize(patterns{dets(i, 5)}, [h w], 'nearest');
    
    xrange = (dets(i, 1) - canvas(1) + 1):(dets(i, 3) - canvas(1) + 1);
    yrange = (dets(i, 2) - canvas(2) + 1):(dets(i, 4) - canvas(2) + 1);
    
    rawptn(yrange, xrange) = ptn;
    
    imptns(:,:,i) = rawptn;
    dets_new(i,:) = [min(xrange) min(yrange) max(xrange) max(yrange)];
end