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
params.visualize = 0;

params.w([2 3 5]) = 4.*[-1 1 1];

temp = [];
for idx = 1:100 % length(dets)
    try
        onedata.idx = idx;
        % [onedata.onedet, onedata.unaries, onedata.pairwise] = prepare_data(num2str(idx));
        onedata = load(fullfile(cachepath, [num2str(idx, '%06d') '.mat']));
        onedata = threshold_detections(onedata, -30);
    catch
        continue;
    end        

    if(0)
        [odet, ndet1, ndet2] = greedy_inference(onedata, params);
    else
        params.w([2 3 5]) = 4.*[-1 1 1];
        params.tw = 1;
        odet = greedy_inference2(onedata, params, 1);
        
        params.w([2 3 5]) = 4.*[-1 1 1];
        params.tw = 1;
        odet2 = greedy_inference_hp(onedata, params);
        
        if(~isempty(odet) && (any(isnan(odet(:, end))) || any(isinf(odet(:, end)))))
            keyboard;
        end

        pick = nms(onedata.onedet, 0.5);
        ndet1 = onedata.onedet(pick, :);

        pick = nms_new(onedata.onedet, 0.6);
        ndet2 = onedata.onedet(pick, :);
    end
    if(0)
        subplot(221);
        if(~isempty(odet))
            show_results(idx, odet(odet(:, end) > 100, :), params); 
        end
        subplot(222);
        if(~isempty(odet2))
            show_results(idx, odet2(odet2(:, end) > 100, :), params); 
        end
        subplot(223);
        if(~isempty(ndet1))
            show_results(idx, ndet1(ndet1(:, end) > -20, :), params); 
        end
        subplot(224);
        if(~isempty(ndet2))
            show_results(idx, ndet2(ndet2(:, end) > -20, :), params); 
        end
        % pause(1);
    end
    
    temp = [temp; idx.*ones(size(odet, 1), 1), odet];
    
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
%
command = ['./evaluate_object ' occpath ' 0.7'];
system(command);
command = ['./evaluate_object ' occpath2 ' 0.7'];
system(command);
command = ['./evaluate_object ' nmspath2 ' 0.7'];
system(command);
command = ['./evaluate_object ' nmspath ' 0.7'];
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
