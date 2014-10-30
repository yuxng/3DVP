unix(['rm ' occpath '/*']);
unix(['rm ' nmspath '/*']);

evallist = [];

% a = 10;
% b = 2;
% c = 10;

params = learn_params(data, dets, {'bias', 0.1, 'w', [a, -b b -c b], 'tw', 0.4, 'visualize', 0});
for idx = 1:3799
    try
        onedata.idx = idx;
        % [onedata.onedet, onedata.unaries, onedata.pairwise] = prepare_data(num2str(idx));
        onedata = load(fullfile(cachepath, [num2str(idx, '%06d') '.mat']));
        
        [odet, ndet] = greedy_inference(onedata, params);
        
        write_kitti_result(fullfile(occpath, [num2str(ids_val(idx), '%06d') '.txt']), odet, data);
        write_kitti_result(fullfile(nmspath, [num2str(ids_val(idx), '%06d') '.txt']), ndet, data);
        
        evallist(end+1) = ids_val(idx);
    catch
    end
end

fp = fopen('kitti_ids_val.txt', 'w');
fprintf(fp, '%d\n', length(evallist));
for i = 1:length(evallist)
    fprintf(fp, '%d\n', evallist(i));
end
fclose(fp);
% 
command = ['./evaluate_object ' occpath];
system(command);
command = ['./evaluate_object ' nmspath];
system(command);

%%
figure
a1 = compute_aps(occpath, '-');
a2 = compute_aps(nmspath, '--');
axis([0 1 0 1])
grid on
legend({['occ easy ' num2str(a1(1), '%.02f')] ...
    ['occ moder ' num2str(a1(2), '%.02f')] ...
    ['occ hard ' num2str(a1(3), '%.02f')] ...
    ['nms easy ' num2str(a2(1), '%.02f')] ...
    ['nms moder ' num2str(a2(2), '%.02f')] ...
    ['nms hard ' num2str(a2(3), '%.02f')]})