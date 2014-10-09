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
color = [0 255 255];
for j = 1:3
    tmp = im(:,:,j);
    tmp(pattern == 3) = color(j);
    im(:,:,j) = tmp;
end
im = uint8(im);  