function pairwise = compute_pairwise_match(dets, params)
% the function takes quite long time 
% we should mex it.

imptns = get_image_patterns(dets, params.pattern);

pairwise = zeros(size(dets, 1), size(dets, 1), 2);
for i = 1:size(dets, 1)
    o = boxoverlap(dets(i+1:end, 1:4), dets(i, 1:4));
    
    idx = find(o > 0);
    idx = idx + i;
    
    % fprintf('\r %d/%d', i, size(dets, 1));
    stdout_withFlush([num2str(i) '/' num2str(size(dets, 1))]);
    for j = idx' % i+1:size(dets, 1)
        pairwise(i, j, :) = compute_match(dets(i, 1:4), dets(j, 1:4), imptns{i}, imptns{j});
        pairwise(j, i, :) = pairwise(i, j, :);
    end
end
% pairwise = sparse(pairwise);
end

function imptns = get_image_patterns(dets, patterns)
imptns = cell(size(dets, 1), 1);

dets(:, 1:4) = round(dets(:, 1:4));

canvas = [min(dets(:, 1)), min(dets(:, 2)), max(dets(:, 3)), max(dets(:, 4))];
for i = 1:size(dets, 1)
    rawptn = zeros(canvas(4) - canvas(2) + 1, canvas(3) - canvas(1) + 1);
    
    h = dets(i, 4) - dets(i, 2) + 1;
    w = dets(i, 3) - dets(i, 1) + 1;
    
    ptn = imresize(patterns{dets(i, 5)}, [h w], 'nearest');
    
    xrange = (dets(i, 1) - canvas(1) + 1):(dets(i, 3) - canvas(1) + 1);
    yrange = (dets(i, 2) - canvas(2) + 1):(dets(i, 4) - canvas(2) + 1);
    
    rawptn(yrange, xrange) = ptn;
    
    imptns{i}.rawptn = sparse(rawptn);
    
    imptns{i}.visibleidx =  find(rawptn == 1);
    imptns{i}.occidx = find(rawptn == 2);
    imptns{i}.truncidx = find(rawptn == 3);
end

end

function scores = compute_match(nearbox, farbox, nearptn, farptn)
% the function compute pairwise potential using pattern matching
% 1. if there is large overlap between visible patterns -> high negative
% 2. if there is compatible visibility pattern -> reward
% 3. no ovelap between region, 0 no edge

% find the order
if(nearbox(4) < farbox(4))
    scores = compute_match(farbox, nearbox, farptn, nearptn);
    return;
end

scores = zeros(1, 1, 2);

if(0) % not faster...
    % visibility overlap
    ia = sum(ismembc(nearptn.visibleidx, farptn.visibleidx)); % sum(nearptn(:) == 1 & farptn(:) == 1);
    a1 = length(nearptn.visibleidx);
    a2 = length(farptn.visibleidx);
    scores(1)  = ia / (a1 + a2 - ia);
    
    % occlusion explained
    ia = sum(ismembc(nearptn.visibleidx, farptn.occidx));
    a2 = length(farptn.visibleidx) + length(farptn.occidx) + length(farptn.truncidx);

    scores(2)  = ia / a2;
else
    % visibility overlap
    ia = sum(nearptn.rawptn(:) == 1 & farptn.rawptn(:) == 1);
    ia = ia + sum(nearptn.rawptn(:) == 2 & farptn.rawptn(:) == 2);
    a1 = sum(nearptn.rawptn(:) == 1) + sum(nearptn.rawptn(:) == 2);
    a2 = sum(farptn.rawptn(:) == 1) + sum(farptn.rawptn(:) == 2);

    scores(1)  = ia / (a1 + a2 - ia);
    % occlusion explained
    ia = sum(nearptn.rawptn(:) == 1 & farptn.rawptn(:) == 2);
    a2 = sum(farptn.rawptn(:) >= 1);

    scores(2)  = ia / a2;
end

end