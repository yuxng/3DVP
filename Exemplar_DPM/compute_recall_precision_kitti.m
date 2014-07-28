% compute recall and viewpoint accuracy
function [recall, precision, ap] = compute_recall_precision_kitti

cls = 'car';

% read ids of validation images
object = load('kitti_ids.mat');
ids = object.ids_val;
M = numel(ids);

% open prediction file
filename = sprintf('kitti_train/%s_test.mat', cls);
object = load(filename);
dets_all = object.dets;

% KITTI path
globals;
pascal_init;
root_dir = KITTIroot;
data_set = 'training';
cam = 2;
label_dir = fullfile(root_dir, [data_set '/label_' num2str(cam)]);

energy = [];
correct = [];
overlap = [];
count = zeros(M,1);
num = zeros(M,1);
num_pr = 0;
for i = 1:M
    fprintf('%s: %d/%d\n', cls, i, M);
    
    % read ground truth bounding box
    img_idx = ids(i);
    objects = readLabels(label_dir, img_idx);
    clsinds = strmatch(cls, lower({objects(:).type}), 'exact');
    n = numel(clsinds);
    bbox = zeros(n, 4);
    for j = 1:n
        bbox(j,:) = [objects(clsinds(j)).x1 objects(clsinds(j)).y1 ...
            objects(clsinds(j)).x2 objects(clsinds(j)).y2];
    end
    count(i) = size(bbox, 1);
    det = zeros(count(i), 1);
    
    % get predicted bounding box
    dets = dets_all{img_idx + 1};    
    if isempty(dets) == 0
        I = nms(dets, 0.5);
        dets = dets(I, :);    
    end
    
    num(i) = size(dets, 1);
    % for each predicted bounding box
    for j = 1:num(i)
        num_pr = num_pr + 1;
        energy(num_pr) = dets(j, 6);        
        bbox_pr = dets(j, 1:4);
        
        % compute box overlap
        if isempty(bbox) == 0
            o = boxoverlap(bbox, bbox_pr);
            [maxo, index] = max(o);
            if maxo >= 0.5 && det(index) == 0
                overlap{num_pr} = index;
                correct(num_pr) = 1;
                det(index) = 1;
            else
                overlap{num_pr} = [];
                correct(num_pr) = 0;
            end
        else
            overlap{num_pr} = [];
            correct(num_pr) = 0;
        end
    end
end
overlap = overlap';

[threshold, index] = sort(energy, 'descend');
correct = correct(index);
n = numel(threshold);
recall = zeros(n,1);
precision = zeros(n,1);
num_correct = 0;
for i = 1:n
    % compute precision
    num_positive = i;
    num_correct = num_correct + correct(i);
    if num_positive ~= 0
        precision(i) = num_correct / num_positive;
    else
        precision(i) = 0;
    end
    
    % compute recall
    recall(i) = num_correct / sum(count);
end

ap = VOCap(recall, precision);
fprintf('AP = %.4f\n', ap);

% draw recall-precision and accuracy curve
figure(1);
hold on;
plot(recall, precision, 'g', 'LineWidth',3);
h = xlabel('Recall');
set(h, 'FontSize', 12);
h = ylabel('Precision');
set(h, 'FontSize', 12);
tit = sprintf('Average Precision = %.1f', 100*ap);
h = title(tit);
set(h, 'FontSize', 12);
hold off;