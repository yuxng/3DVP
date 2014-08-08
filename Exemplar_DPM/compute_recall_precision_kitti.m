% compute recall and viewpoint accuracy
function compute_recall_precision_kitti

cls = 'car';

% read ids of validation images
object = load('kitti_ids.mat');
ids = object.ids_val;
M = numel(ids);

% load data
object = load('../KITTI/data.mat');
data = object.data;

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
correct_view = [];
correct_easy = [];
correct_moderate = [];
count = zeros(M,1);
count_easy = zeros(M,1);
count_moderate = zeros(M,1);
num = zeros(M,1);
num_pr = 0;
for i = 1:M
    fprintf('%s: %d/%d\n', cls, i, M);
    
    % read ground truth bounding box, occlusion and truncation
    img_idx = ids(i);
    objects = readLabels(label_dir, img_idx);
    clsinds = strmatch(cls, lower({objects(:).type}), 'exact');
    n = numel(clsinds);
    bbox = zeros(n, 4);
    occlusion = zeros(n, 1);
    truncation = zeros(n, 1);
    height = zeros(n, 1);
    view_gt = zeros(n, 1);    
    for j = 1:n
        bbox(j,:) = [objects(clsinds(j)).x1 objects(clsinds(j)).y1 ...
            objects(clsinds(j)).x2 objects(clsinds(j)).y2];
        occlusion(j) = objects(clsinds(j)).occlusion;
        truncation(j) = objects(clsinds(j)).truncation;
        height(j) = objects(clsinds(j)).y2 - objects(clsinds(j)).y1;
        azimuth = objects(clsinds(j)).alpha*180/pi;
        if azimuth < 0
            azimuth = azimuth + 360;
        end
        azimuth = azimuth - 90;
        if azimuth < 0
            azimuth = azimuth + 360;
        end
        view_gt(j) = azimuth;        
    end
    count(i) = size(bbox, 1);
    count_easy(i) = sum(occlusion == 0 & truncation < 0.15 & height > 40);
    count_moderate(i) = sum(occlusion < 2 & truncation < 0.3 & height > 25);
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
        
        cid = dets(j, 5);
        azimuth = data.azimuth(cid);
        view_pr = azimuth;        
        
        % compute box overlap
        if isempty(bbox) == 0
            o = boxoverlap(bbox, bbox_pr);
            [maxo, index] = max(o);
            if maxo >= 0.7 && det(index) == 0
                correct(num_pr) = 1;
                det(index) = 1;
                
                amax = max(view_gt(index), view_pr);
                amin = min(view_gt(index), view_pr);
                diff = min(amax - amin, 360 - amax + amin);
                if diff < 15
                    correct_view(num_pr) = 1;
                else
                    correct_view(num_pr) = 0;
                end                
                
                if occlusion(index) == 0 && truncation(index) < 0.15 && height(index) > 40
                    correct_easy(num_pr) = 1;
                else
                    correct_easy(num_pr) = -1;
                end
                
                if occlusion(index) < 2 && truncation(index) < 0.3 && height(index) > 25
                    correct_moderate(num_pr) = 1;
                else
                    correct_moderate(num_pr) = -1;
                end                
            else
                correct(num_pr) = 0;
                correct_view(num_pr) = 0;
                correct_easy(num_pr) = 0;
                correct_moderate(num_pr) = 0;
            end
        else
            correct(num_pr) = 0;
            correct_view(num_pr) = 0;
            correct_easy(num_pr) = 0;
            correct_moderate(num_pr) = 0;
        end
    end
end

[threshold, index] = sort(energy, 'descend');
correct = correct(index);
correct_view = correct_view(index);
correct_easy = correct_easy(index);
correct_moderate = correct_moderate(index);
n = numel(threshold);
recall = zeros(n,1);
precision = zeros(n,1);
accuracy = zeros(n,1);
recall_easy = zeros(n,1);
precision_easy = zeros(n,1);
recall_moderate = zeros(n,1);
precision_moderate = zeros(n,1);
num_correct = 0;
num_correct_view = 0;
num_correct_easy = 0;
num_correct_moderate = 0;
num_positive = 0;
num_positive_easy = 0;
num_positive_moderate = 0;
for i = 1:n
    % compute precision and recall for all
    if correct(i) ~= -1
        num_positive = num_positive + 1;
        num_correct = num_correct + correct(i);
        num_correct_view = num_correct_view + correct_view(i);            
    end
    
    if num_positive ~= 0
        precision(i) = num_correct / num_positive;
    else
        precision(i) = 0;
    end
    if num_positive ~= 0
        accuracy(i) = num_correct_view / num_positive;
    else
        accuracy(i) = 0;
    end     
    recall(i) = num_correct / sum(count);
    
    % compute precision and recall for easy
    if correct_easy(i) ~= -1
        num_positive_easy = num_positive_easy + 1;
        num_correct_easy = num_correct_easy + correct_easy(i);
    end
    if num_positive_easy ~= 0
        precision_easy(i) = num_correct_easy / num_positive_easy;
    else
        precision_easy(i) = 0;
    end
    recall_easy(i) = num_correct_easy / sum(count_easy);
    
    % compute precision and recall for moderate
    if correct_moderate(i) ~= -1
        num_positive_moderate = num_positive_moderate + 1;
        num_correct_moderate = num_correct_moderate + correct_moderate(i);
    end
    if num_positive_moderate ~= 0
        precision_moderate(i) = num_correct_moderate / num_positive_moderate;
    else
        precision_moderate(i) = 0;
    end
    recall_moderate(i) = num_correct_moderate / sum(count_moderate);     
end

ap = VOCap(recall, precision);
fprintf('AP_all = %.4f\n', ap);

aa = VOCap(recall, accuracy);
fprintf('AA = %.4f\n', aa);

ap_easy = VOCap(recall_easy, precision_easy);
fprintf('AP_easy = %.4f\n', ap_easy);

ap_moderate = VOCap(recall_moderate, precision_moderate);
fprintf('AP_moderate = %.4f\n', ap_moderate);

% draw recall-precision and accuracy curve
figure(1);
hold on;
plot(recall, precision, 'r', 'LineWidth',3);
plot(recall_easy, precision_easy, 'g', 'LineWidth',3);
plot(recall_moderate, precision_moderate, 'b', 'LineWidth',3);
h = xlabel('Recall');
set(h, 'FontSize', 12);
h = ylabel('Precision');
set(h, 'FontSize', 12);
tit = sprintf('Car APs');
h = title(tit);
set(h, 'FontSize', 12);
hold off;

save pr_exemplar.mat recall precision recall_easy precision_easy recall_moderate precision_moderate