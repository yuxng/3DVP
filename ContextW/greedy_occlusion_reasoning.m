function greedy_occlusion_reasoning(varargin)

% run occlusion reasoning for all the validation images
if nargin < 1
    varargin = {};
else
    for i = 2:2:length(varargin)
        if(ischar(varargin{i}))
            varargin{i} = str2num(varargin{i});
        end
    end
end

setpath;

load(datafile);
load(detfile);

if is_train
    params = learn_params(data, dets, varargin);
else
    params = learn_params_test(data);
end

N = length(dets);
odets = cell(1, N);
for idx = 1:N
    disp(idx);
    tic;
    onedata.idx = idx;
    [onedata.onedet, onedata.unaries, onedata.pairwise] = prepare_data(num2str(idx), params, data, dets);
    odets{idx} = greedy_inference2(onedata, params, 1);
    toc;
end

filename = fullfile(outpath, 'odets.mat');
save(filename, 'odets', '-v7.3');