function testvalidation(varargin)
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

load(fullfile(datapath, 'data.mat'));
load(detfile);
load(fullfile(datapath, 'kitti_ids_new.mat'));

params = learn_params(data, dets, varargin);

cachepath = fullfile(rootpath, 'ContextW/data/');

cd(fullfile(rootpath, 'ContextW/'));

occpath = 'results/occdet';
nmspath = 'results/occ2det';

for i = 1:length(varargin)
    occpath = [occpath '_' number2string(varargin{i})];
    nmspath = [nmspath '_' number2string(varargin{i})];
end

mkdir(occpath);
mkdir(nmspath);

unix(['rm ' occpath '/*']);
unix(['rm ' nmspath '/*']);
evallist = [];

for idx = 1:length(dets)
    try
        onedata.idx = idx;
        [onedata.onedet, onedata.unaries, onedata.pairwise] = prepare_data(num2str(idx));
        % onedata = load(fullfile(cachepath, [num2str(idx, '%06d') '.mat']));
    catch
        continue;
    end
    
    odet = greedy_inference2(onedata, params, 1);
    pick = nms_new(onedata.onedet, 0.6);
    ndet = onedata.onedet(pick, :);
    % ndet = greedy_inference2(onedata, params, 0);

    write_kitti_result(fullfile(occpath, [num2str(ids_val(idx), '%06d') '.txt']), odet, data);
    write_kitti_result(fullfile(nmspath, [num2str(ids_val(idx), '%06d') '.txt']), ndet, data);

    evallist(end+1) = ids_val(idx);

    stdout_withFlush(num2str(ids_val(idx), '%06d'));
end

% finished all the inference. Do evaluations
fp = fopen(fullfile(occpath, 'kitti_ids_val.txt'), 'w');
fprintf(fp, '%d\n', length(evallist));
for i = 1:length(evallist)
    fprintf(fp, '%d\n', evallist(i));
end
fclose(fp);
command = ['cp ' fullfile(occpath, 'kitti_ids_val.txt') ' ' fullfile(nmspath, 'kitti_ids_val.txt')];
system(command);


figure
% compile  the evaluation code..
system('./compile.sh');

command = ['./evaluate_object ' occpath ' 0.7'];
system(command);
command = ['./evaluate_object ' nmspath ' 0.7'];
system(command);

aps(1, :) = compute_aps(occpath, '-');
aps(2, :) = compute_aps(nmspath, '--');
axis([0 1 0 1])
grid on
legend({['occ easy ' num2str(aps(1,1), '%.02f')] ...
    ['occ moder ' num2str(aps(1,2), '%.02f')] ...
    ['occ hard ' num2str(aps(1,3), '%.02f')] ...
    ['nms easy ' num2str(aps(2,1), '%.02f')] ...
    ['nms moder ' num2str(aps(2,2), '%.02f')] ...
    ['nms hard ' num2str(aps(2,3), '%.02f')]})
saveas(gcf, fullfile(occpath, 'cardet7.fig'));
saveas(gcf, fullfile(occpath, 'cardet7.png'));

figure
command = ['./evaluate_object ' occpath ' 0.5'];
system(command);
command = ['./evaluate_object ' nmspath ' 0.5'];
system(command);

aps(3, :) = compute_aps(occpath, '-');
aps(4, :) = compute_aps(nmspath, '--');
axis([0 1 0 1])
grid on
legend({['occ easy ' num2str(aps(3,1), '%.02f')] ...
    ['occ moder ' num2str(aps(3,2), '%.02f')] ...
    ['occ hard ' num2str(aps(3,3), '%.02f')] ...
    ['nms easy ' num2str(aps(4,1), '%.02f')] ...
    ['nms moder ' num2str(aps(4,2), '%.02f')] ...
    ['nms hard ' num2str(aps(4,3), '%.02f')]})
saveas(gcf, fullfile(occpath, 'cardet7.fig'));
saveas(gcf, fullfile(occpath, 'cardet7.png'));

% clean up and save
save(fullfile(occpath, 'cardet'), 'aps', 'params');
unix(['rm ' occpath '/*.txt']);
unix(['rm ' nmspath '/*.txt']);

close all
return;

end

function str = number2string(num)

if(ischar(num))
    str = num;
elseif(length(num) == 1)
    str = num2str(num);
else
    str = num2str(num(1));
    for i = 2:length(num)
        str = [str ',' num2str(num(i))];
    end
end

end
