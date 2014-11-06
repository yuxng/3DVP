function odet = greedy_inference_hp(data, params)
% similar model as greedy inference 2
% but the pairwise potential is switched to high order potential
% pairwise potential has a problem of double counting!!!
% e.g. occlusion explainted potential can be larger than the penalty
% if there are more than one occluder on the same region...
if(isempty(data.onedet))
    odet = [];
    return;
end

% not using it anymore! yay
% data.unaries(data.unaries(:, 1) < -0.1, 1) = data.unaries(data.unaries(:, 1) < -0.1, 1) .* 3;
threshold = 0; % params.w(1) * -0.6; % 0;

onedet = data.onedet;
unaries = data.unaries;
pairwise = data.pairwise;

imptns = get_image_patterns(onedet, params.pattern);

solution = zeros(size(onedet, 1), 1);
% temp. lets not use too much occluded detections... doesn't make sense
% anyway...
unaries(unaries(:, 2) > 0.9, 2) = 10;

prob = build_problem(unaries, pairwise, params);
u = prob.u;
p = prob.p;

count = 1;
odet = zeros(size(onedet));

canvas = uint8(zeros(size(imptns{1}.rawptn)));

while(1 && any(p(:)))
    if(any(isinf(u)))
        score = u;
        hp = zeros(size(score));
        for i = 1:length(score)
            hp(i) = compute_contextual_match(imptns, onedet, solution(1:count-1, :), i, params);
        end
        score = score + hp;
        % score = u + sum(p(:, isinf(u)), 2);
    else
        score = u;
    end

    [v, idx] = max(score);
    if(v > threshold)
        u(idx) = -Inf;
        % solution(idx) = true;
        solution(count) = idx;
        odet(count, :) = [onedet(idx, 1:5), v];
        count = count + 1;
        
        % mark selected on explained
        if(params.visualize)
            canvas = canvas + uint8(imptns{idx}.rawptn == 1);
            subplot(311);
            show_results(data.idx, odet(1:count-1, :), params);
            subplot(312);
            imagesc(canvas, [0 3]);
            axis equal
        end
        imptns = update_contextual_pattern(imptns, onedet, solution(1:count-1, :), idx);
        if(params.visualize)
            subplot(313);
            imagesc(imptns{idx}.rawptn, [0 3]);
            title(v);
            axis equal
            drawnow;
        end
    else
        break;
    end
end

odet = odet(1:count-1, :);
solution = solution(1:count-1, :);
% append remaining
onedet(solution, :) = [];
score(solution) = [];

if(~isempty(solution))
    onedet(:, end) = score;
end
odet = [odet; onedet];

pick = nms_new(odet, 0.7, length(solution));
[~, idx] = sort(-odet(pick, end));
pick = pick(idx);
odet = odet(pick ,:);

end

function imptns = update_contextual_pattern(imptns, onedet, solution, i)
% replace the explained occlusion region to the visible region

o = boxoverlap(onedet(solution, 1:4), onedet(i, 1:4));
indices = find(o > 0);

ibox = onedet(i, 1:4);

for j = 1:length(indices)
    didx = solution(indices(j));
    if(didx == i)
        % newly selected one
        for k = 1:length(indices)
            didx2 = solution(indices(k));
            if(didx2 == i)
                continue;
            end            
            if(ibox(4) < onedet(didx2, 4))
                % other one is near, so may have been explained.
                ovisidx = imptns{didx2}.visibleidx;
                imptns{i}.rawptn(ovisidx(imptns{i}.rawptn(ovisidx) == 2)) = 1;
            end
        end
    else
        % previous solutions
        if(ibox(4) > onedet(didx, 4))
            % i is near, so may have been explained.
            ivisidx = imptns{i}.visibleidx;
            imptns{didx}.rawptn(ivisidx(imptns{didx}.rawptn(ivisidx) == 2)) = 1;
        end
    end
    
    imptns{didx}.visibleidx = find(imptns{didx}.rawptn == 1);
    imptns{didx}.occidx = find(imptns{didx}.rawptn == 2);
    imptns{didx}.truncidx = find(imptns{didx}.rawptn == 3);
end

end

function [match] = compute_contextual_match(imptns, onedet, solution, i, params)
% high order potential. avoid double counting problem in pairwise form.

if(any(solution == i))
    match = -inf; % ignore already selected one
    return;
end

o = boxoverlap(onedet(solution, 1:4), onedet(i, 1:4));
idx = find(o > 0);

scores = zeros(1, 2);

iptn = imptns{i};
ibox = onedet(i, 1:4);

norm1 = length(iptn.visibleidx) + 0.001;
norm2 = sum(iptn.rawptn(:) > 0) + 0.001;

for j = 1:length(idx)
    other = solution(idx(j));
    
    otherptn = imptns{other}.visibleidx;
    scores(1) = scores(1) + sum(iptn.rawptn(otherptn) == 1) / norm1;
    iptn.rawptn(otherptn(iptn.rawptn(otherptn) == 1)) = 0;
    
    if(ibox(4) < onedet(other, 4))
        % i is far
        otherptn = imptns{other}.visibleidx;
        scores(2) = scores(2) + sum(iptn.rawptn(otherptn) == 2) / norm2; % occlusion explained
        iptn.rawptn(otherptn(iptn.rawptn(otherptn) == 2)) = 0;
    else
        % i is near
        otherptn = imptns{other}.occidx;
        scores(2) = scores(2) + sum(iptn.rawptn(otherptn) == 1) / sum(imptns{other}.rawptn(:) > 0); % occlusion explained
    end
    
    if(0)
        subplot(313)
        show_results(6, onedet([i other], :), params);
        title(scores);
        keyboard;
    end
end
match = params.w(4) * scores(1) + params.w(5) * scores(2);
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
    
    fprintf('\r %d/%d', i, size(dets, 1));
end
fprintf('\n');

end