function greedy_occlusion_reasoning(varargin)

% matlabpool open;

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

is_train = 0;
rootpath = '/scratch/yuxiang/Projects/SLM/';
if is_train
    datafile = fullfile(rootpath, 'KITTI/data.mat');
    detfile = fullfile(rootpath, 'ACF/kitti_train_ap_125/car_3d_aps_125_combined_test.mat');
    outpath = fullfile(rootpath, 'ContextW/kitti_train_ap_125/');
else
    datafile = fullfile(rootpath, 'KITTI/data_kitti.mat');
    detfile = fullfile(rootpath, 'ACF/kitti_test_acf_3d_227_flip/car_3d_ap_227_combined_test.mat');
    outpath = fullfile(rootpath, 'ContextW/kitti_test_ap_227/');
end 

object = load(datafile);
data = object.data;
object = load(detfile);
dets = object.dets;

if is_train
    params = learn_params(data, dets, varargin);
else
    params = learn_params_test(data);
end
clear data;

N = length(dets);
odets = cell(1, N);
parfor idx = 1:N
    disp(idx);
    tic;

    % prepare data
    filename = fullfile(outpath, [num2str(idx, '%06d') '.mat']);
    if exist(filename) ~= 0
        object = load(filename, 'onedet', 'unaries', 'pairwise');
        onedet = object.onedet;
        unaries = object.unaries;
        pairwise = object.pairwise;
    else
        onedet = dets{idx};
        if(isempty(onedet))
            unaries = zeros(0, 3);
            pairwise = zeros(0, 0);
        else
            rt = box2rect(onedet(:, 1:4));
            onedet(:, 1:4) = onedet(:, 1:4) + (params.transform(onedet(:, 5), :) .* [rt(:, 3:4) rt(:, 3:4)]);
            unaries = compute_unaries(onedet, params);
            pairwise = compute_pairwise_match_new(onedet, params);
        end
        parsave(filename, idx, onedet, unaries, pairwise);
    end
    
    onedata = [];
    onedata.idx = idx;
    onedata.onedet = onedet;
    onedata.unaries = unaries;
    onedata.pairwise = pairwise;
    
    odets{idx} = greedy_inference2(onedata, params, 1);
    toc;
end

filename = fullfile(outpath, 'odets.mat');
save(filename, 'odets', '-v7.3');

matlabpool close;


function parsave(filename, idx, onedet, unaries, pairwise)

save(filename, 'idx', 'onedet', 'unaries', 'pairwise');