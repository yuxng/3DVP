function [onedet, unaries, pairwise] = prepare_data(idx, params, data, dets)

savefile = 0;
if(ischar(idx))
    idx = str2double(idx);
    savefile = 1;
end

setpath;

try
    load(fullfile(outpath, [num2str(idx, '%06d') '.mat']), 'idx', 'onedet', 'unaries', 'pairwise');
    return;
catch
end

if nargin < 3
    load(fullfile(datapath, 'data.mat'));
end

if nargin < 4
    load(detfile);
end

if nargin < 2
    params = learn_params(data, dets);
end

onedet = dets{idx};
clear dets;

if(~exist(outpath, 'dir'))
    mkdir(outpath);
end

%% get the full boxes
if(isempty(onedet))
    unaries = zeros(0, 3);
    pairwise = zeros(0, 0);
else
    rt = box2rect(onedet(:, 1:4));
    onedet(:, 1:4) = onedet(:, 1:4) + (params.transform(onedet(:, 5), :) .* [rt(:, 3:4) rt(:, 3:4)]);

    if(params.ver == 0.5) % not using... just complicated and not helping...
        error('dont use');
        unaries = compute_unaries2(onedet, params);
        pairwise = compute_pairwise_match2(onedet, unaries, params);
    else
        unaries = compute_unaries(onedet, params);
        pairwise = compute_pairwise_match_new(onedet, params);
    end
end

if savefile
    save(fullfile(outpath, [num2str(idx, '%06d') '.mat']), 'idx', 'onedet', 'unaries', 'pairwise');
end

if(0) % debugging
    temp = load(fullfile(outpath, [num2str(idx, '%06d') '.mat']), 'idx', 'onedet', 'unaries', 'pairwise');
    assert(all(temp.unaries(:) == unaries(:)));
    assert(all(temp.pairwise(:) == pairwise(:)));
end

end