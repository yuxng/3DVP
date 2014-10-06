function exemplar_display_result_pascal

cls = 'car';
threshold = -20;
is_train = 1;
threshold_overlap = 0.5;
result_dir = 'data';

% read detection results
filename = sprintf('%s/%s_test.mat', result_dir, cls);
object = load(filename);
dets = object.dets;
fprintf('load detection done\n');

% PASCAL path
exemplar_globals;
if is_train
    ids = textread(sprintf(VOCopts.imgsetpath, 'val'), '%s');
else
    ids = textread(sprintf(VOCopts.imgsetpath, 'test'), '%s');
end
N = numel(ids);

figure;
for i = 1:N
    disp(ids{i});
    
    % read ground truth bounding box
    if is_train
        rec = PASreadrecord(sprintf(VOCopts.annopath, ids{i}));
        objects = rec.objects;
        clsinds = strmatch(cls, {objects(:).class}, 'exact');
        n = numel(clsinds);
        bbox = zeros(n, 4); 
        for j = 1:n
            bbox(j,:) = objects(clsinds(j)).bbox;     
        end
        flags_gt = zeros(n, 1);
    end
    
    % get predicted bounding box
    det = dets{i};
    if isempty(det) == 1
        fprintf('no detection for image %d\n', i);
        continue;
    end
    if max(det(:,6)) < threshold
        fprintf('maximum score %.2f is smaller than threshold\n', max(det(:,6)));
        continue;
    end
    if isempty(det) == 0
        I = nms_new(det, threshold_overlap);
        det = det(I, :);    
    end
    num = size(det, 1);
    
    % for each predicted bounding box
    if is_train
        flags_pr = zeros(num, 1);
        for j = 1:num
            bbox_pr = det(j, 1:4);  

            % compute box overlap
            if isempty(bbox) == 0
                o = boxoverlap(bbox, bbox_pr);
                [maxo, index] = max(o);
                if maxo >= 0.5 && flags_gt(index) == 0
                    flags_pr(j) = 1;
                    flags_gt(index) = 1;
                end
            end
        end
    end
    
    file_img = sprintf(VOCopts.imgpath, ids{i});
    I = imread(file_img);
    
    % show all the detections
    figure(1);
    imshow(I);
    hold on;
    
    for k = 1:size(dets{i},1)
        if dets{i}(k,6) > threshold
            % get predicted bounding box
            bbox_pr = dets{i}(k,1:4);
            bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
            rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
        end
    end
    hold off;

    figure(2);
    imshow(I);
    hold on;
    for k = 1:num
        if det(k,6) > threshold
            % get predicted bounding box
            bbox_pr = det(k,1:4);
            bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
            if is_train
                if flags_pr(k)
                    rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
                else
                    rectangle('Position', bbox_draw, 'EdgeColor', 'r', 'LineWidth', 2);
                end
            else
                rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
            end
            text(bbox_pr(1), bbox_pr(2), num2str(det(k,5)), 'FontSize', 16, 'BackgroundColor', 'r');
        end
    end
    
    if is_train
        for k = 1:n
            if flags_gt(k) == 0
                bbox_gt = bbox(k,1:4);
                bbox_draw = [bbox_gt(1), bbox_gt(2), bbox_gt(3)-bbox_gt(1), bbox_gt(4)-bbox_gt(2)];
                rectangle('Position', bbox_draw, 'EdgeColor', 'y', 'LineWidth', 2);
            end
        end
    end
    hold off;
    pause;
end