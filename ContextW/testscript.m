clear

setpath;

load(fullfile(datapath, 'data.mat'));
load(detfile);
load(fullfile(datapath, 'kitti_ids_new.mat'));

params = learn_params(data, dets);
%%
idx = 6;
onedata.idx = idx;
[onedata.onedet, onedata.unaries, onedata.pairwise] = prepare_data(num2str(idx));
odet = greedy_inference(onedata, params);