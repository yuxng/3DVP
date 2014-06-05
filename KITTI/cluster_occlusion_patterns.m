% clustering of occlusion patterns
function cluster_occlusion_patterns(dirname)

cnum = 64;
cnum_2 = 16;
% list the occlusion patterns
files = dir(fullfile(dirname, '*.jpg'));

% read the occlusion patterns
num = numel(files);
patterns = cell(num, 1);
widths = zeros(num, 1);
heights = zeros(num, 1);
for i = 1:num
    filename = fullfile(dirname, files(i).name);
    I = imread(filename);
    patterns{i} = rgb2gray(I);
    widths(i) = size(I,2);
    heights(i) = size(I,1);
end

% construct the data matrix for clustering
w = round(mean(widths));
h = round(mean(heights));
data = uint8(zeros(h*w, num));
for i = 1:num
    I = patterns{i};
    im = imresize(I, [h w], 'bilinear');
    im = reshape(im, h*w, 1);
    data(:, i) = im;
end

[centers, assignments] = vl_ikmeans(data, cnum);

% visualize cluster centers
% figure(1);
% for i = 1:cnum
%     I = centers(:, i);
%     I = reshape(I, h, w);
%     subplot(8,8,i);
%     imagesc(uint8(I));
%     axis equal;
%     axis off;
% end

[c, assignments] = vl_ikmeans(uint8(centers), cnum_2);
figure(1);
for i = 1:cnum_2
    I = c(:, i);
    I = reshape(I, h, w);
    subplot(4,4,i);
    imagesc(uint8(I));
    axis equal;
    axis off;
end