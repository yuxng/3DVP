function greedy_write_kitti_results

setpath;

% load detections
filename = fullfile(outpath, 'odets.mat');
object = load(filename);
odets = object.odets;

% load data
object = load(datafile);
data = object.data;
object = load(fullfile(datapath, 'kitti_ids_new.mat'));
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end

N = numel(odets);
for i = 1:N
    if is_train
        filename = fullfile('results_kitti_train', [num2str(ids(i), '%06d') '.txt']);
    else
        filename = fullfile('results_kitti_test', [num2str(ids(i), '%06d') '.txt']);
    end
    disp(filename);
    write_kitti_result(filename, odets{i}, data);
end

if is_train
    command = './evaluate_object results_kitti_train 0.7';
    system(command);

    filename = 'results_kitti_train/plot/car_detection.txt';
    data = load(filename);

    recall = data(:,1);
    precision_easy = data(:,2);
    precision_moderate = data(:,3);
    precision_hard = data(:,4);

    ap_easy = VOCap(recall, precision_easy);
    fprintf('AP_easy = %.4f\n', ap_easy);

    ap_moderate = VOCap(recall, precision_moderate);
    fprintf('AP_moderate = %.4f\n', ap_moderate);

    ap = VOCap(recall, precision_hard);
    fprintf('AP_hard = %.4f\n', ap);

    % draw recall-precision and accuracy curve
    figure(1);
    hold on;
    plot(recall, precision_easy, 'g', 'LineWidth',3);
    plot(recall, precision_moderate, 'b', 'LineWidth',3);
    plot(recall, precision_hard, 'r', 'LineWidth',3);
    h = xlabel('Recall');
    set(h, 'FontSize', 12);
    h = ylabel('Precision');
    set(h, 'FontSize', 12);
    tit = sprintf('Car APs');
    h = title(tit);
    set(h, 'FontSize', 12);
    hold off;
    
    % pose estimation
    filename = 'results_kitti_train/plot/car_orientation.txt';
    data = load(filename);

    recall = data(:,1);
    precision_easy = data(:,2);
    precision_moderate = data(:,3);
    precision_hard = data(:,4);

    ap_easy = VOCap(recall, precision_easy);
    fprintf('AOS_easy = %.4f\n', ap_easy);

    ap_moderate = VOCap(recall, precision_moderate);
    fprintf('AOS_moderate = %.4f\n', ap_moderate);

    ap = VOCap(recall, precision_hard);
    fprintf('AOS_hard = %.4f\n', ap);

    % draw recall-precision and accuracy curve
    figure(2);
    hold on;
    plot(recall, precision_easy, 'g', 'LineWidth',3);
    plot(recall, precision_moderate, 'b', 'LineWidth',3);
    plot(recall, precision_hard, 'r', 'LineWidth',3);
    h = xlabel('Recall');
    set(h, 'FontSize', 12);
    h = ylabel('Precision');
    set(h, 'FontSize', 12);
    tit = sprintf('Car APs');
    h = title(tit);
    set(h, 'FontSize', 12);
    hold off;    
end