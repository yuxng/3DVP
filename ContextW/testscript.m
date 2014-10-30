clear

setpath;

load(fullfile(datapath, 'data.mat'));
load(detfile);
load(fullfile(datapath, 'kitti_ids_new.mat'));

params = learn_params(data, dets);
%%
cachepath = fullfile(rootpath, 'ContextW/data/');

occpath = 'occdet';
nmspath = 'nmsdet';


wall = [10 2 10; ];
aps = {};
for i = 1:size(wall, 1)
    a = wall(i, 1);
    b = wall(i, 2);
    c = wall(i, 3);
    
    onebatch;
    
    aps{i} = a1;
end