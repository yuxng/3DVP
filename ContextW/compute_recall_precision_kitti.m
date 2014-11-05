% compute recall and viewpoint accuracy
function compute_recall_precision_kitti

cls = 'car';

% read detection results
result_dir = 'kitti_train_ap_125';
filename = sprintf('%s/odets.mat', result_dir);
object = load(filename);
dets_3d = object.odets;
fprintf('load detection done\n');

% read ids of validation images
object = load('kitti_ids_new.mat');
ids = object.ids_val;
M = numel(ids);

% KITTI path
exemplar_globals;
root_dir = KITTIroot;
data_set = 'training';
cam = 2;
label_dir = fullfile(root_dir, [data_set '/label_' num2str(cam)]);

energy = [];
correct = [];
correct_easy = [];
correct_moderate = [];
correct_hard = [];
count = zeros(M,1);
count_easy = zeros(M,1);
count_moderate = zeros(M,1);
count_hard = zeros(M,1);
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
    translation = zeros(n, 3);
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
        translation(j,:) = objects(clsinds(j)).t;
    end
    count(i) = size(bbox, 1);
    count_easy(i) = sum(occlusion == 0 & truncation < 0.15 & height > 40);
    count_moderate(i) = sum(occlusion < 2 & truncation < 0.3 & height > 25);
    count_hard(i) = sum(occlusion < 3 & truncation < 0.5 & height > 25); 
    det = zeros(count(i), 1);
    
    % get predicted bounding box
    dets = dets_3d{i};
    
%     if isempty(dets) == 0
%         I = nms(dets, 0.5);
%         dets = dets(I, :);    
%     end
    
    num(i) = size(dets, 1);
    
    % sort detections
    if num(i)
        [~, index] = sort(dets(:,end), 'descend');
        dets = dets(index,:);
    end
    
    % for each predicted bounding box
    for j = 1:num(i)
        num_pr = num_pr + 1;
        energy(num_pr) = dets(j, 6);        
        bbox_pr = dets(j, 1:4);
        
        % compute box overlap
        if isempty(bbox) == 0
            o = boxoverlap(bbox, bbox_pr);
            [maxo, index] = max(o);
            if maxo >= 0.7 && det(index) == 0
                correct(num_pr) = 1;
                det(index) = 1;
                
                if occlusion(index) == 0 && truncation(index) < 0.15 && height(index) > 40
                    correct_easy(num_pr) = 1;
                else
                    correct_easy(num_pr) = -1;
                end
                
                if occlusion(index) < 2 && truncation(index) < 0.3 && height(index) > 15
                    correct_moderate(num_pr) = 1;
                else
                    correct_moderate(num_pr) = -1;
                end
                
                if occlusion(index) < 3 && truncation(index) < 0.5 && height(index) > 15
                    correct_hard(num_pr) = 1;
                else
                    correct_hard(num_pr) = -1;
                end                
                
            else
                correct(num_pr) = 0;
                correct_easy(num_pr) = 0;
                correct_moderate(num_pr) = 0;
                correct_hard(num_pr) = 0;
            end
        else
            correct(num_pr) = 0;
            correct_easy(num_pr) = 0;
            correct_moderate(num_pr) = 0;
            correct_hard(num_pr) = 0;
        end
    end
end

[threshold, index] = sort(energy, 'descend');
correct = correct(index);
correct_easy = correct_easy(index);
correct_moderate = correct_moderate(index);
correct_hard = correct_hard(index);
n = numel(threshold);
recall = zeros(n,1);
precision = zeros(n,1);
recall_easy = zeros(n,1);
precision_easy = zeros(n,1);
recall_moderate = zeros(n,1);
precision_moderate = zeros(n,1);
recall_hard = zeros(n,1);
precision_hard = zeros(n,1);
num_correct = 0;
num_correct_easy = 0;
num_correct_moderate = 0;
num_correct_hard = 0;
num_positive = 0;
num_positive_easy = 0;
num_positive_moderate = 0;
num_positive_hard = 0;
for i = 1:n
    % compute precision and recall for all
    num_positive = num_positive + 1;
    num_correct = num_correct + correct(i);
    if num_positive ~= 0
        precision(i) = num_correct / num_positive;
    else
        precision(i) = 0;
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
    
    % compute precision and recall for hard
    if correct_hard(i) ~= -1
        num_positive_hard = num_positive_hard + 1;
        num_correct_hard = num_correct_hard + correct_hard(i);
    end
    if num_positive_hard ~= 0
        precision_hard(i) = num_correct_hard / num_positive_hard;
    else
        precision_hard(i) = 0;
    end
    recall_hard(i) = num_correct_hard / sum(count_hard);    
end

ap = VOCap(recall, precision);
fprintf('AP_all = %.4f\n', ap);

ap_easy = VOCap(recall_easy, precision_easy);
fprintf('AP_easy = %.4f\n', ap_easy);

ap_moderate = VOCap(recall_moderate, precision_moderate);
fprintf('AP_moderate = %.4f\n', ap_moderate);

ap_hard = VOCap(recall_hard, precision_hard);
fprintf('AP_hard = %.4f\n', ap_hard);

% draw recall-precision and accuracy curve
figure(1);
hold on;
plot(recall, precision, 'y', 'LineWidth',3);
plot(recall_easy, precision_easy, 'g', 'LineWidth',3);
plot(recall_moderate, precision_moderate, 'b', 'LineWidth',3);
plot(recall_hard, precision_hard, 'r', 'LineWidth',3);
h = xlabel('Recall');
set(h, 'FontSize', 12);
h = ylabel('Precision');
set(h, 'FontSize', 12);
tit = sprintf('Car APs');
h = title(tit);
set(h, 'FontSize', 12);
hold off;