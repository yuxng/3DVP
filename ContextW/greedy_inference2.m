function odet = greedy_inference2(data, params, nmstype)

data.unaries(data.unaries(:, 1) < -0.1, 1) = data.unaries(data.unaries(:, 1) < -0.1, 1) .* 3;


threshold = 0;

onedet = data.onedet;
unaries = data.unaries;
pairwise = data.pairwise;

if(isempty(onedet))
    odet = [];
    return;
end

solution = zeros(size(onedet, 1), 1);

% temp. lets not use too much occluded detections... doesn't make sense
% anyway...
unaries(unaries(:, 2) > 0.9, 2) = 10;

prob = build_problem(unaries, pairwise, params);
u = prob.u;
p = prob.p;

count = 1;
odet = zeros(size(onedet));
while(1 && any(p(:)))
    if(any(u < -100000))
        score = u + sum(p(:, u < -100000), 2);
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
    else
        break;
    end
end
odet = odet(1:count-1, :);
solution = solution(1:count-1, :);

tempd = recompute_score(onedet, solution, prob.u, prob.p);
tempd(solution, :) = []; % rescore the remainder

odet(:, end) = odet(:, end) + 1000; % keep the MAP solution high ranked
odet = [odet; tempd];

if(nmstype)
    % pick = nms_new(odet, 0.7, length(solution));
    pick = nms_new(odet, 0.7);
    [~, idx] = sort(-odet(pick, end));
    pick = pick(idx);
    odet = odet(pick ,:);
else
    p = pairwise(:,:,1);
    p11 = p(solution, :);
    p11 = p11(:, solution);
    p12 = p(solution, :);
    p12(:, solution) = [];
    p21 = p(:, solution);
    p21(solution, :) = [];
    p22 = p;
    p22(solution, :) = [];
    p22(:, solution) = [];
    
    p = [p11, p12; p21, p22];
    if(size(odet, 1) ~= size(p, 1))
        keyboard;
    end
    
    pick = nms_pairwise(odet, p, 0.5, length(solution));
    [~, idx] = sort(-odet(pick, end));
    pick = pick(idx);
    odet = odet(pick ,:);
    %%
%     if(0)
%         imageidx = data.idx;
%         load ../KITTI/kitti_ids_new.mat
% 
%         im = show_image(ids_val, imageidx);
%         top = 1:size(odet, 1);
% 
%         imshow(im);
%         for i = 1:length(top)
%             im = draw_mask(im, odet(top(i), :), params.pattern);
%             imshow(im);
%             % rectangle('position', box2rect(odet(top(i), 1:4)), 'linewidth', 2, 'edgecolor', 'r');
%             drawnow;
%         end
%         title('Occlusion Reasoning')
% 
%         pause;
%     end
end

% if(~all(ismember(1:length(solution), pick)))
%     keyboard;
% end

end