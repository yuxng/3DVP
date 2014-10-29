function [bbox_new, pattern_new] = exemplar_transform_truncated_box(bbox, pattern)

w = bbox(3) - bbox(1) + 1;
h = bbox(4) - bbox(2) + 1;    

index = find(pattern == 1 | pattern == 2);
[y, x] = ind2sub(size(pattern), index);
cx = size(pattern, 2)/2;
cy = size(pattern, 1)/2;
width = size(pattern, 2);
height = size(pattern, 1);                 
pattern_visible = pattern(min(y):max(y), min(x):max(x));

% find the object center
sx = w / size(pattern_visible, 2);
sy = h / size(pattern_visible, 1);
tx = bbox(1) - sx*min(x);
ty = bbox(2) - sy*min(y);
cx = sx * cx + tx;
cy = sy * cy + ty;
width = sx * width;
height = sy * height;
bbox_new = [cx-width/2 cy-height/2 cx+width/2 cy+height/2];

if(nargout == 2)
    pattern_new = imresize(pattern, [height width], 'nearest');
end