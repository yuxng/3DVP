function demo_greedy(dets_3d, id)

addpath(genpath('../KITTI'));

% KITTI path
globals;
root_dir = KITTIroot;
data_set = 'training';
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% read image
img_idx = id - 1;  
file_img = sprintf('%s/%06d.png',image_dir, img_idx);
I = imread(file_img);
width = size(I, 2);
height = size(I, 1);

% sort detections
objects = dets_3d{img_idx + 1};
num = numel(objects);
scores = zeros(1, num);
for i = 1:num
    scores(i) = objects(i).score;
end
[~, index] = sort(scores, 'descend');
objects = objects(index);

% compute the distances and patterns
distances = zeros(1, num);
patterns = uint8(zeros(height, width, num));
dets = zeros(num, 6);
for i = 1:num
    distances(i) = norm(objects(i).t);
    pattern = objects(i).pattern;
    h = size(pattern, 1);
    w = size(pattern, 2);
    x = max(1, floor(objects(i).x1));
    y = max(1, floor(objects(i).y1));
    patterns(y:y+h-1, x:x+w-1, i) = pattern;
    dets(i,:) = [objects(i).x1 objects(i).y1 objects(i).x2 objects(i).y2 ...
            objects(i).cid objects(i).score];    
end

% add objects into the scene greedily
flags = zeros(1, num);
for i = 1:num
    pi = patterns(:,:,i);
    di = distances(i);
    flags(i) = 1;
    for j = 1:i-1
        % for each objects in the scene
        if flags(j) == 1
            pj = patterns(:,:,j);
            dj = distances(j);
            % compute the matching score of occlusion patterns
            index = pi > 0 & pj > 0;
            overlap = sum(sum(index));
            r1 = overlap / sum(sum(pi > 0));
            r2 = overlap / sum(sum(pj > 0));
            if r1 < 0.1 || r2 < 0.1
                s = 1;
            else
                if di > dj
                    s = (sum(pi(index) == 2) / overlap) * (sum(pi(index) == 1) / overlap);
                else
                    s = (sum(pi(index) == 1) / overlap) * (sum(pi(index) == 2) / overlap);
                end
            end
            if s < 0.25  % incompatitable
                flags(i) = 0;
                break;
            end
        end
    end
end    

% plot 2D detections
figure;

% show non-maximum suppression results
subplot(2, 1, 1);
imshow(I);
hold on;

if isempty(dets) == 0
    index = nms(dets, 0.5);
    dets = dets(index, :);
end

til = sprintf('%d', i);
for k = 1:size(dets, 1)
    % get predicted bounding box
    bbox = dets(k,1:4);
    bbox_draw = [bbox(1), bbox(2), bbox(3)-bbox(1), bbox(4)-bbox(2)];
    rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);            
    text(bbox(1), bbox(2), num2str(k), 'FontSize', 16, 'BackgroundColor', 'r');
    til = sprintf('%s, s%d=%.2f', til, k, dets(k,6));
end
title(til);
hold off;
xlabel('Non Maximum Suppression');

% show occlusion reasoning results
subplot(2, 1, 2);

for k = 1:num
    if flags(k) == 0
        continue;
    end    
    % get predicted bounding box
    bbox = [objects(k).x1 objects(k).y1 objects(k).x2 objects(k).y2];       
    im = create_mask_image(objects(k).pattern);
    h = size(im, 1);
    w = size(im, 2);
    x = max(1, floor(bbox(1)));
    y = max(1, floor(bbox(2)));
    Isub = I(y:y+h-1, x:x+w-1, :);
    index = im == 255;
    im(index) = Isub(index);
    I(y:y+h-1, x:x+w-1, :) = uint8(0.1*Isub + 0.9*im);        
end  

imshow(I);
hold on;
til = sprintf('%d', i);
for k = 1:num
    if flags(k) == 0
        continue;
    end
    % get predicted bounding box
    bbox = [objects(k).x1 objects(k).y1 objects(k).x2 objects(k).y2];
    bbox_draw = [bbox(1), bbox(2), bbox(3)-bbox(1), bbox(4)-bbox(2)];
    rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);            
    text(bbox(1), bbox(2), num2str(k), 'FontSize', 16, 'BackgroundColor', 'r');
    til = sprintf('%s, s%d=%.2f', til, k, objects(k).score);       
end
title(til);
hold off;
xlabel('Greedy Occlusion Reasoning');

function im = create_mask_image(pattern)

% 2D occlusion mask
im = 255*ones(size(pattern,1), size(pattern,2), 3);
color = [0 255 0];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 1) = color(j);
    im(:,:,j) = tmp;
end
color = [255 0 0];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 2) = color(j);
    im(:,:,j) = tmp;
end
im = uint8(im); 