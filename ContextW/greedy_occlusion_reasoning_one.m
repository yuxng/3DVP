function greedy_occlusion_reasoning_one(varargin)

% run occlusion reasoning for all the validation images
if nargin < 1
    varargin = {};
else
    for i = 2:2:length(varargin)
        if(ischar(varargin{i}))
            varargin{i} = str2num(varargin{i});
        end
    end
end

is_train = 0;
threshold = -2;
rootpath = '/home/yuxiang/Projects/SLM/';
if is_train
    datafile = fullfile(rootpath, 'KITTI/data.mat');
    detfile = fullfile(rootpath, 'ACF/kitti_train_ap_125/car_3d_aps_125_combined_test.mat');
    outpath = fullfile(rootpath, 'ContextW/kitti_train_ap_125/');
    image_dir = '/home/yuxiang/Projects/KITTI_Dataset/data_object_image_2/training/image_2';
else
    datafile = fullfile(rootpath, 'KITTI/data_kitti.mat');
    detfile = fullfile(rootpath, 'ACF/kitti_test_acf_3d_227_flip/car_3d_ap_227_combined_test.mat');
    outpath = fullfile(rootpath, 'ContextW/kitti_test_ap_227/');
    image_dir = '/home/yuxiang/Projects/KITTI_Dataset/data_object_image_2/testing/image_2';
end 

object = load(datafile);
data = object.data;
object = load(detfile);
dets = object.dets;
object = load('kitti_ids_new.mat');
if is_train
    ids = object.ids_val;
else
    ids = object.ids_test;
end

if is_train
    params = learn_params(data, dets, varargin);
else
    params = learn_params_test(data);
end

N = length(dets);
for idx = 1:N
    disp(idx);

    % prepare data
    filename = fullfile(outpath, [num2str(idx, '%06d') '.mat']);
    if exist(filename) ~= 0
        object = load(filename, 'onedet', 'unaries', 'pairwise');
        onedet = object.onedet;
        unaries = object.unaries;
        pairwise = object.pairwise;
    else
        onedet = dets{idx};
        if(isempty(onedet))
            unaries = zeros(0, 3);
            pairwise = zeros(0, 0);
        else
            rt = box2rect(onedet(:, 1:4));
            onedet(:, 1:4) = onedet(:, 1:4) + (params.transform(onedet(:, 5), :) .* [rt(:, 3:4) rt(:, 3:4)]);
            unaries = compute_unaries(onedet, params);
            pairwise = compute_pairwise_match_new(onedet, params);
        end
        parsave(filename, idx, onedet, unaries, pairwise);
    end
    
    onedata = [];
    onedata.idx = idx;
    onedata.onedet = onedet;
    onedata.unaries = unaries;
    onedata.pairwise = pairwise;
    
    det = greedy_inference2(onedata, params, 1);
    
    % show result
    img_idx = ids(idx);
    file_img = sprintf('%s/%06d.png', image_dir, img_idx);
    I = imread(file_img);
    
    hf = figure(1);
    % add pattern
    num = size(det, 1);
    for k = 1:num
        if det(k,6) > threshold
            bbox_pr = det(k,1:4);
            bbox = zeros(1,4);
            bbox(1) = max(1, floor(bbox_pr(1)));
            bbox(2) = max(1, floor(bbox_pr(2)));
            bbox(3) = min(size(I,2), floor(bbox_pr(3)));
            bbox(4) = min(size(I,1), floor(bbox_pr(4)));
            w = bbox(3) - bbox(1) + 1;
            h = bbox(4) - bbox(2) + 1;

            % apply the 2D occlusion mask to the bounding box
            % check if truncated pattern
            cid = det(k,5);
            pattern = data.pattern{cid};                
            index = find(pattern == 1);
            if data.truncation(cid) > 0 && isempty(index) == 0                
                [y, x] = ind2sub(size(pattern), index);
                cx = size(pattern, 2)/2;
                cy = size(pattern, 1)/2;
                width = size(pattern, 2);
                height = size(pattern, 1);                 
                pattern = pattern(min(y):max(y), min(x):max(x));

                % find the object center
                sx = w / size(pattern, 2);
                sy = h / size(pattern, 1);
                tx = bbox(1) - sx*min(x);
                ty = bbox(2) - sy*min(y);
                cx = sx * cx + tx;
                cy = sy * cy + ty;
                width = sx * width;
                height = sy * height;
                bbox_pr = round([cx-width/2 cy-height/2 cx+width/2 cy+height/2]);
                width = bbox_pr(3) - bbox_pr(1) + 1;
                height = bbox_pr(4) - bbox_pr(2) + 1;
                
                pattern = imresize(data.pattern{cid}, [height width], 'nearest');
                
                bbox = zeros(1,4);
                bbox(1) = max(1, floor(bbox_pr(1)));
                start_x = bbox(1) - floor(bbox_pr(1)) + 1;
                bbox(2) = max(1, floor(bbox_pr(2)));
                start_y = bbox(2) - floor(bbox_pr(2)) + 1;
                bbox(3) = min(size(I,2), floor(bbox_pr(3)));
                bbox(4) = min(size(I,1), floor(bbox_pr(4)));
                w = bbox(3) - bbox(1) + 1;
                h = bbox(4) - bbox(2) + 1;
                pattern = pattern(start_y:start_y+h-1, start_x:start_x+w-1);
            else
                pattern = imresize(pattern, [h w], 'nearest');
            end
            
            % build the pattern in the image
            height = size(I,1);
            width = size(I,2);
            P = uint8(zeros(height, width));
            x = bbox(1);
            y = bbox(2);
            index_y = y:min(y+h-1, height);
            index_x = x:min(x+w-1, width);
            P(index_y, index_x) = pattern(1:numel(index_y), 1:numel(index_x));
            
            % show segments
            dispColor = [0 255 0];
            scale = round(max(size(I))/500);            
            [gx, gy] = gradient(double(P));
            g = gx.^2 + gy.^2;
            g = conv2(g, ones(scale), 'same');
            edgepix = find(g > 0);
            npix = numel(P);
            for b = 1:3
                I((b-1)*npix+edgepix) = dispColor(b);
            end            
            
            % show occluded region
            im = create_occlusion_image(pattern);
            x = bbox(1);
            y = bbox(2);
            Isub = I(y:y+h-1, x:x+w-1, :);
            index = im == 255;
            im(index) = Isub(index);
            I(y:y+h-1, x:x+w-1, :) = uint8(0.1*Isub + 0.9*im);            
        end
    end
    
    imshow(I);
    hold on;
    for k = 1:num
        if det(k,6) > threshold
            % get predicted bounding box
            bbox_pr = det(k,1:4);
%             bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
%             if is_train
%                 if flags_pr(k)
%                     rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
%                 else
%                     rectangle('Position', bbox_draw, 'EdgeColor', 'r', 'LineWidth', 2);
%                 end
%             else
%                 rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
%             end
            s = sprintf('%.2f', det(k,6));
            text(bbox_pr(1), bbox_pr(2), s, 'FontSize', 8, 'BackgroundColor', 'c');
        end
    end    
    pause;
end


function parsave(filename, idx, onedet, unaries, pairwise)

save(filename, 'idx', 'onedet', 'unaries', 'pairwise');


function im = create_occlusion_image(pattern)

% 2D occlusion mask
im = 255*ones(size(pattern,1), size(pattern,2), 3);
color = [0 0 255];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 2) = color(j);
    im(:,:,j) = tmp;
end
color = [0 255 255];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 3) = color(j);
    im(:,:,j) = tmp;
end
im = uint8(im); 