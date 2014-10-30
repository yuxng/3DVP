function [odet, ndet] = greedy_inference(data, params)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% params.bias = 0.0; % about -20
% % bias = 0.2; % about -20
% 
% 
% a = 10;
% b = 0.6;
% c = 0.5;
% 
% params.w(1) = a;
% params.w(2) = -b;
% params.w(3) = b;
% params.w(4) = -c;
% params.w(5) = b;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
threshold = 0;
visualize = params.visualize;

onedet = data.onedet;
unaries = data.unaries;
pairwise = data.pairwise;

if(isempty(onedet))
    return;
end

solution = false(size(onedet, 1), 1);

% temp. lets not use too much occluded detections... doesn't make sense
% anyway...
unaries(unaries(:, 2) > 0.9, 2) = 10;

prob = build_problem(unaries, pairwise, params);
u = prob.u;
p = prob.p;

count = 1;
odet = zeros(size(onedet));

while(1 && any(p(:)))
    if(any(solution))
        score = u + sum(p(:, solution), 2);
    else
        score = u;
    end

    [v, idx] = max(score);
    if(v > threshold)
        solution(idx) = true;
        u(idx) = -Inf;
        
        odet(count, :) = [onedet(idx, 1:5), v];
        count = count + 1;
    else
        break;
    end
end

odet = odet(1:count-1, :);

if(visualize)
    vizscore = -0.05;
    
    imageidx = data.idx;
    load ../KITTI/kitti_ids_new.mat
    
    subplot(311)
    im = show_image(ids_val, imageidx);
    top = 1:size(odet, 1);

    imshow(im);
    for i = 1:length(top)
        if(odet(top(i), end) > vizscore * params.w(1) + params.bias)
            im = draw_mask(im, odet(top(i), :), params.pattern);
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


pick = nms_new(onedet, 0.6);
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