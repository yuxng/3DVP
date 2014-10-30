function params = learn_params(data, dets, vargins)

params = struct('w', [], 'bias', [], 'b', [], 'pattern', [], 'centers', [], 'transform', [], 'snorm', []);

centers = unique(data.idx_ap);
centers(centers == -1) = [];

% compute pattern penalization cost
b = sparse(length(data.pattern), 1);
b(centers) = compute_unary_panalization(data.pattern(centers));

% get the box transforms
transform = zeros(length(data.pattern), 4);
for i = 1:length(centers)
    if(any(data.pattern{centers(i)}(:) == 3))
        dummybox = [1 1 100 100]; w = 100; h = 100;
        bboxnew = exemplar_transform_truncated_box(dummybox, data.pattern{centers(i)});
        
        transform(centers(i), :) = (bboxnew - dummybox) ./ [w h w h];
    end
end
%% normalize the scores
minscore = 0;
maxscore = 0;
for i = 1:length(dets)
    if(isempty(dets{i}))
        continue;
    end
    minscore = min(minscore, min(dets{i}(:, end)));
    maxscore = max(maxscore, max(dets{i}(:, end)));
end
%% get the parameters
params.w = zeros(1, 5);
params.bias = 0;

params.b = b;
params.transform = transform;
params.centers = centers;
params.pattern = data.pattern;
params.snorm = max(abs(maxscore), max(minscore));

if(nargin >= 3)
    for i = 1:2:length(vargins)
        params.(vargins{i}) = vargins{i+1};
    end
end

end