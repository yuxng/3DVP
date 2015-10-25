function show_segmentation

opt = globals();
root_dir = opt.path_kitti_root;
data_set = 'training';
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]);

% annotations
path_ann = 'Annotations_new';
files = dir(fullfile(path_ann, '*.mat'));
N = numel(files);

figure(1);
cmap = colormap(summer);
for i = 1:N
    % load annotation
    filename = fullfile(path_ann, files(i).name);
    disp(filename);
    object = load(filename);
    record = object.record;
    objects = record.objects;
    
    % load image
    filename = sprintf('%s/%06d.png',image_dir, i - 1);
    I = imread(filename);    
    
    num = numel(objects);
    for j = 1:num
        object = objects(j);
        if strcmp(object.type, 'Car') == 1
            
            % bounding box
            bbox_gt = [object.x1 object.y1 object.x2 object.y2];
            bbox = zeros(1,4);
            bbox(1) = max(1, floor(bbox_gt(1)));
            bbox(2) = max(1, floor(bbox_gt(2)));
            bbox(3) = min(size(I,2), floor(bbox_gt(3)));
            bbox(4) = min(size(I,1), floor(bbox_gt(4)));
            w = bbox(3) - bbox(1) + 1;
            h = bbox(4) - bbox(2) + 1;
            
            % apply the 2D occlusion mask to the bounding box
            % check if truncated pattern
            pattern = object.pattern;              
            index = find(pattern == 1);
            if object.truncation > 0 && isempty(index) == 0
                
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
                
                pattern = imresize(object.pattern, [height width], 'nearest');
                
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
            
            % show occluded region
            im = create_occlusion_image(pattern);
            Isub = I(y:y+h-1, x:x+w-1, :);
            index = im == 255;
            im(index) = Isub(index);
            I(y:y+h-1, x:x+w-1, :) = uint8(0.1*Isub + 0.9*im);             
            
            % show segments
            index_color = 1 + floor((j-1) * size(cmap,1) / num);
            dispColor = 255*cmap(index_color,:);
            scale = round(max(size(I))/400);            
            [gx, gy] = gradient(double(P));
            g = gx.^2 + gy.^2;
            g = conv2(g, ones(scale), 'same');
            edgepix = find(g > 0);
            npix = numel(P);
            for b = 1:3
                I((b-1)*npix+edgepix) = dispColor(b);
            end 
        end
    end
    
    imshow(I);
    hold off;
    pause;
end

function im = create_occlusion_image(pattern)

% 2D occlusion mask
im = 255*ones(size(pattern,1), size(pattern,2), 3);
color = [255 0 0];
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