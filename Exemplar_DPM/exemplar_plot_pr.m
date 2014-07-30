function exemplar_plot_pr

globals;
pascal_init;

% load results
pr_dpm = load('../3rd_party/voc-release4.01/pr.mat');
pr_exemplar = load('pr_exemplar.mat');
pr_greedy = load('pr_greedy.mat');

figure(1);
hold on;
recall = pr_dpm.recall_easy;
precision = pr_dpm.precision_easy;
plot(recall, precision, 'r', 'LineWidth', 3);
axis equal;
xlim([0 1]);
ylim([0 1]);
h = xlabel('Recall');
set(h, 'FontSize', 12);
h = ylabel('Precision');
set(h, 'FontSize', 12);
h = title('Car Easy');
set(h, 'FontSize', 12);

ap_dpm_easy = VOCap(recall, precision);
fprintf('AP_easy = %.4f\n', ap_dpm_easy);
s = sprintf('DPM %.1f%%', 100*ap_dpm_easy);
h = legend(s, 'Location', 'SouthWest');
set(h, 'FontSize', 12);

figure(2);
hold on;
recall = pr_dpm.recall_moderate;
precision = pr_dpm.precision_moderate;
plot(recall, precision, 'r', 'LineWidth', 3);
axis equal;
xlim([0 1]);
ylim([0 1]);
h = xlabel('Recall');
set(h, 'FontSize', 12);
h = ylabel('Precision');
set(h, 'FontSize', 12);
h = title('Car Moderate');
set(h, 'FontSize', 12);

ap_dpm_moderate = VOCap(recall, precision);
fprintf('AP_moderate = %.4f\n', ap_dpm_moderate);
s = sprintf('DPM %.1f%%', 100*ap_dpm_moderate);
h = legend(s, 'Location', 'SouthWest');
set(h, 'FontSize', 12);

figure(3);
hold on;
recall = pr_dpm.recall;
precision = pr_dpm.precision;
plot(recall, precision, 'r', 'LineWidth', 3);
axis equal;
xlim([0 1]);
ylim([0 1]);
h = xlabel('Recall');
set(h, 'FontSize', 12);
h = ylabel('Precision');
set(h, 'FontSize', 12);
h = title('Car Hard');
set(h, 'FontSize', 12);

ap_dpm_all = VOCap(recall, precision);
fprintf('AP_all = %.4f\n', ap_dpm_all);
s = sprintf('DPM %.1f%%', 100*ap_dpm_all);
h = legend(s, 'Location', 'SouthWest');
set(h, 'FontSize', 12);


pause;

figure(1);
recall = pr_exemplar.recall_easy;
precision = pr_exemplar.precision_easy;
plot(recall, precision, 'b', 'LineWidth', 3);

ap_exemplar_easy = VOCap(recall, precision);
fprintf('AP_easy = %.4f\n', ap_exemplar_easy);
s1 = sprintf('DPM %.1f%%', 100*ap_dpm_easy);
s2 = sprintf('Exemplar %.1f%%', 100*ap_exemplar_easy);
h = legend(s1, s2, 'Location', 'SouthWest');
set(h, 'FontSize', 12);

figure(2);
recall = pr_exemplar.recall_moderate;
precision = pr_exemplar.precision_moderate;
plot(recall, precision, 'b', 'LineWidth', 3);

ap_exemplar_moderate = VOCap(recall, precision);
fprintf('AP_moderate = %.4f\n', ap_exemplar_moderate);
s1 = sprintf('DPM %.1f%%', 100*ap_dpm_moderate);
s2 = sprintf('Exemplar %.1f%%', 100*ap_exemplar_moderate);
h = legend(s1, s2, 'Location', 'SouthWest');
set(h, 'FontSize', 12);

figure(3);
recall = pr_exemplar.recall;
precision = pr_exemplar.precision;
plot(recall, precision, 'b', 'LineWidth', 3);

ap_exemplar_all = VOCap(recall, precision);
fprintf('AP_all = %.4f\n', ap_exemplar_all);
s1 = sprintf('DPM %.1f%%', 100*ap_dpm_all);
s2 = sprintf('Exemplar %.1f%%', 100*ap_exemplar_all);
h = legend(s1, s2, 'Location', 'SouthWest');
set(h, 'FontSize', 12);

pause;


figure(1);
recall = pr_greedy.recall_easy;
precision = pr_greedy.precision_easy;
plot(recall, precision, 'g', 'LineWidth', 3);

ap_greedy_easy = VOCap(recall, precision);
fprintf('AP_easy = %.4f\n', ap_greedy_easy);
s1 = sprintf('DPM %.1f%%', 100*ap_dpm_easy);
s2 = sprintf('Exemplar %.1f%%', 100*ap_exemplar_easy);
s3 = sprintf('Greedy %.1f%%', 100*ap_greedy_easy);
h = legend(s1, s2, s3, 'Location', 'SouthWest');
set(h, 'FontSize', 12);

figure(2);
recall = pr_greedy.recall_moderate;
precision = pr_greedy.precision_moderate;
plot(recall, precision, 'g', 'LineWidth', 3);

ap_greedy_moderate = VOCap(recall, precision);
fprintf('AP_moderate = %.4f\n', ap_greedy_moderate);
s1 = sprintf('DPM %.1f%%', 100*ap_dpm_moderate);
s2 = sprintf('Exemplar %.1f%%', 100*ap_exemplar_moderate);
s3 = sprintf('Greedy %.1f%%', 100*ap_greedy_moderate);
h = legend(s1, s2, s3, 'Location', 'SouthWest');
set(h, 'FontSize', 12);

figure(3);
recall = pr_greedy.recall;
precision = pr_greedy.precision;
plot(recall, precision, 'g', 'LineWidth', 3);

ap_greedy_all = VOCap(recall, precision);
fprintf('AP_all = %.4f\n', ap_greedy_all);
s1 = sprintf('DPM %.1f%%', 100*ap_dpm_all);
s2 = sprintf('Exemplar %.1f%%', 100*ap_exemplar_all);
s3 = sprintf('Greedy %.1f%%', 100*ap_greedy_all);
h = legend(s1, s2, s3, 'Location', 'SouthWest');
set(h, 'FontSize', 12);