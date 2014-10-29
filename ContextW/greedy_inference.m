function [odet] = greedy_inference(data, params)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
bias = 0.13; % about -20
threshold = 0;

a = 1;
b = 1;
c = 10;
params.w(1) = a;
params.w(2) = -b;
params.w(3) = b;
params.w(4) = -c;
params.w(5) = b;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
odet = [];

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

u = [unaries(:, 1) + bias + 0.1, unaries(:, 2:3)] * params.w(1:3)';
% p = pairwise(:,:,1) * params.w(4) + pairwise(:,:,2) * params.w(5);
% p = (pairwise(:,:,1) > 0.3) * -8;
% p = p + (pairwise(:,:,1) > 0.5) * -8;
% p = p + (pairwise(:,:,1) > 0.7) * -8;
p = pairwise(:,:,1) * params.w(4);
p = p + pairwise(:,:,2) * params.w(5);

while(1)
    if(any(solution))
        score = u + sum(p(:, solution), 2);
    else
        score = u;
    end

    [v, idx] = max(score);
    if(v > threshold)
        solution(idx) = true;
        u(idx) = -Inf;
        
        odet = [odet; onedet(idx, 1:5), v];
    else
        break;
    end
end

%%%
imageidx = data.idx;
load ../KITTI/kitti_ids_new.mat

figure(1)
subplot(211)
im = show_image(ids_val, imageidx);
[~, top] = sort(-odet(:, end));
top = 1:size(odet, 1);

imshow(im);
for i = 1:length(top)
    im = draw_mask(im, odet(top(i), :), params.pattern);
    imshow(im);
    % rectangle('position', box2rect(odet(top(i), 1:4)), 'linewidth', 2, 'edgecolor', 'r');
    pause(0.5);
end
title('Occlusion Reasoning')

subplot(212)
im = show_image(ids_val, imageidx);
imshow(im);

pick = nms_new(onedet, 0.6);
[~, idx] = sort(-onedet(pick, end));
pick = pick(idx);
for i = 1:length(pick)
    if(onedet(pick(i), end) > (-bias + threshold) * params.snorm)
        % rectangle('position', box2rect(onedet(pick(i), 1:4)), 'linewidth', 2, 'edgecolor', 'r');
        im = draw_mask(im, onedet(pick(i), :), params.pattern);
        imshow(im);
        pause(0.5);
    end
end
title('NMS')

return;


saveas(gcf, fullfile('temp/', [num2str(imageidx, '%06d') '.png']));

w = params.w;

imidx = ids_val(imageidx);
save(fullfile('temp/', [num2str(imageidx, '%06d') '.mat']), 'imidx', 'onedet', 'unaries', 'pairwise', 'solution', 'pick', 'w', 'bias');
end