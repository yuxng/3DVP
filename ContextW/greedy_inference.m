function [odet, odet2, ndet] = greedy_inference(data, params)
%%%%%% temporary
data.unaries(data.unaries(:, 1) < -0.1, 1) = data.unaries(data.unaries(:, 1) < -0.1, 1) .* 3;
%%%%%%%%%%%%%%%%%%%%%%
threshold = 0;
visualize = params.visualize;

onedet = data.onedet;
unaries = data.unaries;
pairwise = data.pairwise;

if(isempty(onedet))
    return;
end

% temp. lets not use too much occluded detections... doesn't make sense
% anyway...
unaries(unaries(:, 2) > 0.9, 2) = 10;

prob = build_problem(unaries, pairwise, params);
u = prob.u;
p = prob.p;

count = 1;
odet = zeros(size(onedet));
count2 = 1;
odet2 = zeros(size(onedet));

debug = 0;
if(debug)
    load ../KITTI/kitti_ids_new.mat
    imageidx = data.idx;
    im = show_image(ids_val, imageidx);
end

solution = zeros(size(onedet, 1), 1);
while(1 && any(p(:)))
    if(any(u < -100000))
        score = u + sum(p(:, u < -100000), 2);
    else
        score = u;
    end

    [v, idx] = max(score);
    if(v > 0) % -0.1 * params.w(1))
        if(debug)
            subplot(311)
            im = draw_mask(im, onedet(idx, :), params.pattern);
            imshow(im);
            title(num2str(v));
            fprintf('%.02f, %.02f\n', u(idx), sum(p(idx, solution)));
            keyboard;
        end
        
        u(idx) = -Inf;
        if(v > threshold)
            solution(count) = idx;
            odet(count, :) = [onedet(idx, 1:5), v];
            count = count + 1;
        end
        
        odet2(count2, :) = [onedet(idx, 1:5), v];
        count2 = count2 + 1;
    else
        break;
    end
end
solution(count:end) = [];

odet = odet(1:count-1, :);
odet2 = odet2(1:count2-1, :);

if(0)
    odet2(:, end) = odet2(:, end) + 1000;
    odet2 = [odet2; onedet];

    pick = nms_new(odet2, 0.7);
    [~, idx] = sort(-odet2(pick, end));
    pick = pick(idx);
    odet2 = odet2(pick ,:);
else
    tempd = recompute_score(onedet, solution, prob.u, prob.p);
    tempd(solution, :) = []; % rescore the remainder
    
    odet2(:, end) = odet2(:, end) + 1000; % keep the MAP solution
    odet2 = [odet2; tempd];
    
    pick = nms_new(odet2, 0.8);
    [~, idx] = sort(-odet2(pick, end));
    pick = pick(idx);
    odet2 = odet2(pick ,:);
end


if(visualize)
    vizscore = 0; % -0.05;
    
    imageidx = data.idx;
    load ../KITTI/kitti_ids_new.mat
    
    subplot(311)
    im = show_image(ids_val, imageidx);
    top = 1:size(odet2, 1);

    imshow(im);
    for i = 1:length(top)
        if(odet2(top(i), end) > vizscore)
            im = draw_mask(im, odet2(top(i), :), params.pattern);
            imshow(im);
            % rectangle('position', box2rect(odet(top(i), 1:4)), 'linewidth', 2, 'edgecolor', 'r');
            drawnow;
        end
    end
    title('Occlusion Reasoning')
end

odet = recompute_score(onedet, solution, prob.u, prob.p);
pick = nms_new(odet, 0.7);
odet = odet(pick, :);
%%%
if(visualize)
    subplot(312)
    im = show_image(ids_val, imageidx);
    [~, top] = sort(-odet(:, end));
    % top = 1:size(odet, 1);

    imshow(im);
    for i = 1:length(top)
        if(odet(top(i), end) > vizscore * params.w(1)  + params.bias)
            im = draw_mask(im, odet(top(i), :), params.pattern);
            imshow(im);
            % rectangle('position', box2rect(odet(top(i), 1:4)), 'linewidth', 2, 'edgecolor', 'r');
            drawnow;
        end
    end
    title('Occlusion Reasoning')
end


pick = nms(onedet, 0.5);
[~, idx] = sort(-onedet(pick, end));
pick = pick(idx);
ndet = onedet(pick ,:);

if(visualize)
    vizscore = -20;
    
    subplot(313)
    im = show_image(ids_val, imageidx);
    imshow(im);

    for i = 1:length(pick)
        if(onedet(pick(i), end) > vizscore)
            % rectangle('position', box2rect(onedet(pick(i), 1:4)), 'linewidth', 2, 'edgecolor', 'r');
            im = draw_mask(im, onedet(pick(i), :), params.pattern);
            imshow(im);
            drawnow;
        end
    end
    title('NMS')
    drawnow;
	pause(0.5);
end

return;


saveas(gcf, fullfile('temp/', [num2str(imageidx, '%06d') '.png']));

w = params.w;

imidx = ids_val(imageidx);
save(fullfile('temp/', [num2str(imageidx, '%06d') '.mat']), 'imidx', 'onedet', 'unaries', 'pairwise', 'solution', 'pick', 'w', 'bias');
end