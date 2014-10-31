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
% allscores = [];
for i = 1:length(dets)
    if(isempty(dets{i}))
        continue;
    end
    minscore = min(minscore, min(dets{i}(:, end)));
    maxscore = max(maxscore, max(dets{i}(:, end)));
    % allscores = [allscores ; dets{i}(:, end)];
end
%% get the parameters
params.w = [10 -2 2 -10 2];
params.bias = 0.1;
params.visualize = 0;
params.tw = 0.4;

params.b = b;
params.transform = transform;
params.centers = centers;
params.pattern = data.pattern;
params.snorm = max(abs(maxscore), max(minscore));
% params.ver = 0.5; % not using... unnecessarily complicated
params.ver = 0;
if(params.ver >= 0.5)
    % plot these to understand using pwlinear
    params.lambda1 = [0, 0.1, 0.4, 0.9, 1; 0, 0.5, 1.66, 0, 0; 0, -0.05, -0.5, 1, 0];
    params.lambda2 = [0, 0.5, 0.8, 1; 1, 1.6, 0, 0; 0, -0.3, 1, 0];
    
    params.tw = 0;
end

if(nargin >= 3)
    for i = 1:2:length(vargins)
        if(strcmp(vargins{i}, 'occw'))
            params.w(2) = -vargins{i+1};
            params.w(3) = vargins{i+1};
            params.w(5) = vargins{i+1};
        elseif(strcmp(vargins{i}, 'pairw'))
            params.w(4) = -vargins{i+1};
        else
            params.(vargins{i}) = vargins{i+1};
        end
    end
end

end
