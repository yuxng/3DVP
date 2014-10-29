function [onedet, unaries, pairwise] = prepare_data(idx, params)
savefile = 0;
if(ischar(idx))
    idx = str2double(idx);
    savefile = 1;
end

setpath;

outpath = fullfile(rootpath, 'ContextW/data/');
load(fullfile(rootpath, 'KITTI/data.mat'));
load(fullfile(rootpath, 'ACF/data/car_3d_ap_125_combined_test.mat'));

try
    load(fullfile(outpath, [num2str(idx, '%06d') '.mat']), 'idx', 'onedet', 'unaries', 'pairwise');
    return
catch
end

if nargin < 2
    params = learn_params(data, dets);
end

onedet = dets{idx};
clear dets;
%% get the full boxes
if(isempty(onedet))
    unaries = zeros(0, 3);
    pairwise = zeros(0, 0);
else
    rt = box2rect(onedet(:, 1:4));
    onedet(:, 1:4) = onedet(:, 1:4) + (params.transform(onedet(:, 5), :) .* [rt(:, 3:4) rt(:, 3:4)]);

    unaries = compute_unaries(onedet, params);
    pairwise = compute_pairwise_match(onedet, params);
end

if savefile
    save(fullfile(outpath, [num2str(idx, '%06d') '.mat']), 'idx', 'onedet', 'unaries', 'pairwise');
end

end