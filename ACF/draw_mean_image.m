function draw_mean_image(cid)

% exemplar_globals
SLMroot = '../';
imroot = '/net/skyserver10/workplace/yxiang/KITTI_Dataset/data_object_image_2/training/image_2/';
%%%%%%%%%%%%%%%%%%

load(fullfile(SLMroot, 'KITTI/data.mat'));
% data.bbox = round(data.bbox);
% for i = 1:size(data.bbox, 1)
%     if(data.is_flip(i))
%         % not using 
%         continue;
%     end
%     im = imread(fullfile(imroot, data.imgname{i}));
%     % imshow(im);
%     % rectangle('position', box2rect(data.bbox(:,i)'));
%     patch = subarray(im, data.bbox(2, i), data.bbox(4, i), data.bbox(1, i), data.bbox(3, i));
%     imshow(patch);
%     pause;
% end

centers = unique(data.idx_ap);

tidx = find(data.idx_ap == centers(cid));
oboxes = data.bbox(:, tidx)';
[boxes, mar] = convert_aspect_ratio(oboxes);
boxes = round(boxes);

rheight = 200;
mimages = zeros(rheight, round(rheight*mar), 3);
gimages = zeros(rheight, round(rheight*mar), 1);
nimages = zeros(rheight, round(rheight*mar), 3);

imsize = size(mimages);
count = 0;

for i = 1:length(tidx)
    if(data.is_flip(tidx(i)))
        % not using 
        continue;
    end
    im = imread(fullfile(imroot, data.imgname{tidx(i)}));
    patch = subarray(im, boxes(i, 2), boxes(i, 4), boxes(i, 1), boxes(i, 3));
    patch = imresize(patch, imsize(1:2));
    
    mimages = mimages + double(patch);
    
    [gx, gy] = gradient(double(rgb2gray(patch)));
    gimages = gimages + sqrt(gx.^2 + gy.^2);
    
    npatch = normalize_patch(patch);
    
    nimages = nimages + npatch;
    
    count = count + 1;
end

figure(1); imshow(uint8(mimages ./ count));
figure(2); imshow(uint8(gimages ./ count .* 8));
figure(3); imshow(uint8(nimages ./ count .* 50)  + 128);
disp(count);

end

function patch = normalize_patch(patch)

patch = double(patch);
for i = 1:size(patch, 3)
    pix = patch(:, :, i);
    pix = pix(:);
    
    mpix = mean(pix(pix > 0));
    spix = std(pix(pix > 0));
    
    patch(:, :, i) = (patch(:, :, i) - mpix) ./ spix;
end

end

function B = subarray(A, i1, i2, j1, j2)
dim = size(A);
B = uint8(zeros(i2-i1+1, j2-j1+1, dim(3)));
for i = max(i1,1):min(i2,dim(1))
    for j = max(j1,1):min(j2,dim(2))
        B(i-i1+1, j-j1+1, :) = A(i, j, :); 
    end 
end
end

function [boxes, mar] = convert_aspect_ratio(boxes)

rts = box2rect(boxes);

ar = rts(:, 3) ./ rts(:, 4);
mar = median(ar);

boxes(ar >= mar, :) = expand_box(rts(ar >= mar, :), mar, 1);
boxes(ar < mar, :) = expand_box(rts(ar < mar, :), mar, 0);


end

function boxes = expand_box(rects, ar, vertical)
if(vertical)
    cy = rects(:, 2) + rects(:, 4) ./ 2;
    h = rects(:, 3) ./ ar;
    
    rects(:, 2) = cy - h ./ 2;
    rects(:, 4) = h;
    
    boxes = rect2box(rects);
else
    cx = rects(:, 1) + rects(:, 3) ./ 2;
    w = rects(:, 4) .* ar;
    
    rects(:, 1) = cx - w ./ 2;
    rects(:, 3) = w;
    
    boxes = rect2box(rects);
end

end

function boxes = rect2box(rts)

boxes = rts;
boxes(:, 3) = rts(:, 3) + rts(:, 1) - 1;
boxes(:, 4) = rts(:, 4) + rts(:, 2) - 1;

end

function rts = box2rect(boxes)

rts = boxes;
rts(:, 3) = rts(:, 3) - rts(:, 1) + 1;
rts(:, 4) = rts(:, 4) - rts(:, 2) + 1;

end