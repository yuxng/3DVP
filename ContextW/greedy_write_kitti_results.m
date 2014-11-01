function greedy_write_kitti_results

setpath;

% load detections
filename = fullfile(outpath, 'odets.mat');
object = load(filename);
odets = object.odets;

% load data
object = load(fullfile(datapath, 'data.mat'));
data = object.data;
object = load(fullfile(datapath, 'kitti_ids_new.mat'));
ids_val = object.ids_val;

N = numel(odets);
for i = 1:N
    filename = fullfile('results_kitti_train', [num2str(ids_val(i), '%06d') '.txt']);
    disp(filename);
    write_kitti_result(filename, odets{i}, data);
end

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