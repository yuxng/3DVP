function pairwise = compute_pairwise_match2(dets, unaries, params)
% the function takes quite long time 
% we should mex it.

imptns = get_image_patterns(dets, params.pattern);

pairwise = zeros(size(dets, 1), size(dets, 1), 2);
faridx = zeros(size(dets, 1), size(dets, 1));
for i = 1:size(dets, 1)
    o = boxoverlap(dets(i+1:end, 1:4), dets(i, 1:4));
    
    idx = find(o > 0);
    idx = idx + i;
    
    stdout_withFlush([num2str(i) '/' num2str(size(dets, 1))]);
    for j = idx' % i+1:size(dets, 1)
        [temp, temp2] = compute_match(dets(i, 1:4), dets(j, 1:4), imptns{i}, imptns{j});
        pairwise(i, j, :) = temp;
        pairwise(j, i, :) = temp;
        
        if(temp2 == 1)
            temp2 = j;
        else
            temp2 = i;
        end
        faridx(i, j) = temp2;
        faridx(j, i) = temp2;
    end
end
clear imptns
pairwise(:, :, 1) = pwlinear(pairwise(:, :, 1), params.lambda1);

idx = pairwise(:,:,2) > 0;
temp = pairwise(:,:,2);
temp(idx) = pwlinear(temp(idx), params.lambda2) .* unaries(faridx(idx), 2);
pairwise(:,:,2) = temp;

end

function imptns = get_image_patterns(dets, patterns)
imptns = cell(size(dets, 1), 1);

dets(:, 1:4) = round(dets(:, 1:4));

canvas = [min(dets(:, 1)), min(dets(:, 2)), max(dets(:, 3)), max(dets(:, 4))];
for i = 1:size(dets, 1)
    rawptn = uint8(zeros(canvas(4) - canvas(2) + 1, canvas(3) - canvas(1) + 1));
    
    h = dets(i, 4) - dets(i, 2) + 1;
    w = dets(i, 3) - dets(i, 1) + 1;
    
    ptn = imresize(patterns{dets(i, 5)}, [h w], 'nearest');
    
    xrange = (dets(i, 1) - canvas(1) + 1):(dets(i, 3) - canvas(1) + 1);
    yrange = (dets(i, 2) - canvas(2) + 1):(dets(i, 4) - canvas(2) + 1);
    
    rawptn(yrange, xrange) = ptn;
    
    % rawptn = imresize(rawptn, 1/2); 
    % if we need further speed up.. lets
    % resize the image to make it faster
    
    imptns{i}.rawptn = rawptn;
    
    imptns{i}.visibleidx =  find(rawptn == 1);
    imptns{i}.occidx = find(rawptn == 2);
    imptns{i}.truncidx = find(rawptn == 3);
end

end

function [scores, faridx] = compute_match(nearbox, farbox, nearptn, farptn)
% the function compute pairwise potential using pattern matching
% 1. if there is large overlap between visible patterns -> high negative
% 2. if there is compatible visibility pattern -> reward
% 3. no ovelap between region, 0 no edge

% find the order
if(nearbox(4) < farbox(4))
    scores = compute_match(farbox, nearbox, farptn, nearptn);
    faridx = 2;
    return;
end

scores = zeros(1, 1, 2);

% faster!

% visibility overlap
ia = sum(nearptn.rawptn(farptn.visibleidx) == 1);
ia = ia + sum(nearptn.rawptn(farptn.occidx) == 2);
ia = ia + sum(nearptn.rawptn(farptn.truncidx) == 3);

a1 = length(nearptn.visibleidx) + length(nearptn.occidx) + length(nearptn.truncidx);
a2 = length(farptn.visibleidx) + length(farptn.occidx) + length(farptn.truncidx);

scores(1)  = ia / min(a1, a2);

% percentage of occlusion explained
ia = sum(nearptn.rawptn(farptn.occidx) == 1);
a2 = length(farptn.occidx)  + length(farptn.truncidx);
if(ia > 0)
    scores(2)  = ia / a2;
end

faridx = 1;

end