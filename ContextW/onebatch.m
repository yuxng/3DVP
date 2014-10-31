mkdir(occpath);
mkdir(occpath2);
mkdir(nmspath);
mkdir(nmspath2);

unix(['rm ' occpath '/*']);
unix(['rm ' occpath2 '/*']);
unix(['rm ' nmspath '/*']);
unix(['rm ' nmspath2 '/*']);
evallist = [];

params = learn_params(data, dets);
params.bias = 0.1 * params.w(1);
params.tw = 0.4;
params.visualize = 1;

for idx = 1:length(dets)
    try
        onedata.idx = idx;
        % [onedata.onedet, onedata.unaries, onedata.pairwise] = prepare_data(num2str(idx));
        onedata = load(fullfile(cachepath, [num2str(idx, '%06d') '.mat']));
    catch
        continue;
    end        

    if(0)
        [odet, ndet1, ndet2] = greedy_inference(onedata, params);
    else
        odet = greedy_inference2(onedata, params, 1);
        odet2 = greedy_inference2(onedata, params, 0);

        pick = nms(onedata.onedet, 0.5);
        ndet1 = onedata.onedet(pick, :);

        pick = nms_new(onedata.onedet, 0.6);
        ndet2 = onedata.onedet(pick, :);
    end
    write_kitti_result(fullfile(occpath, [num2str(ids_val(idx), '%06d') '.txt']), odet, data);
    write_kitti_result(fullfile(occpath2, [num2str(ids_val(idx), '%06d') '.txt']), odet2, data);
    write_kitti_result(fullfile(nmspath, [num2str(ids_val(idx), '%06d') '.txt']), ndet1, data);
    write_kitti_result(fullfile(nmspath2, [num2str(ids_val(idx), '%06d') '.txt']), ndet2, data);

    evallist(end+1) = ids_val(idx);
end
%
fp = fopen(fullfile(occpath, 'kitti_ids_val.txt'), 'w');
fprintf(fp, '%d\n', length(evallist));
for i = 1:length(evallist)
    fprintf(fp, '%d\n', evallist(i));
end
fclose(fp);

command = ['cp ' fullfile(occpath, 'kitti_ids_val.txt') ' ' fullfile(occpath2, 'kitti_ids_val.txt')];
system(command);
command = ['cp ' fullfile(occpath, 'kitti_ids_val.txt') ' ' fullfile(nmspath2, 'kitti_ids_val.txt')];
system(command);
command = ['cp ' fullfile(occpath, 'kitti_ids_val.txt') ' ' fullfile(nmspath, 'kitti_ids_val.txt')];
system(command);
%%
command = ['./evaluate_object ' occpath ' 0.5'];
system(command);
command = ['./evaluate_object ' occpath2 ' 0.5'];
system(command);
command = ['./evaluate_object ' nmspath2 ' 0.5'];
system(command);
command = ['./evaluate_object ' nmspath ' 0.5'];
system(command);

%
figure
a1 = compute_aps(occpath, '-');
a2 = compute_aps(occpath2, '-.');
a3 = compute_aps(nmspath, ':');
a4 = compute_aps(nmspath2, '--');
axis([0 1 0 1])
grid on
legend({['occ easy ' num2str(a1(1), '%.02f')] ...
    ['occ moder ' num2str(a1(2), '%.02f')] ...
    ['occ hard ' num2str(a1(3), '%.02f')] ...
    ['occ2 easy ' num2str(a2(1), '%.02f')] ...
    ['occ2 moder ' num2str(a2(2), '%.02f')] ...
    ['occ2 hard ' num2str(a2(3), '%.02f')] ...
    ['nms easy ' num2str(a3(1), '%.02f')] ...
    ['nms moder ' num2str(a3(2), '%.02f')] ...
    ['nms hard ' num2str(a3(3), '%.02f')] ...
    ['nms_{new} easy ' num2str(a4(1), '%.02f')] ...
    ['nms_{new} moder ' num2str(a4(2), '%.02f')] ...
    ['nms_{new} hard ' num2str(a4(3), '%.02f')]})