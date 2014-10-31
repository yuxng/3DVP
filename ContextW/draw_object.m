function im = draw_object(im, det, pattern, color)

det(1:4) = round(det(1:4));

mask = zeros(size(im));
    
h = det(4) - det(2) + 1;
w = det(3) - det(1) + 1;

ptn = imresize(pattern{det(5)}, [h w], 'nearest');

xrange = det(1):det(3);
yrange = det(2):det(4);

xi = find(xrange > 0 & xrange <= size(im, 2));
yi = find(yrange > 0 & yrange <= size(im, 1));

mask(yrange(yi), xrange(xi), 1) = (ptn(yi, xi) == 1) .* color(1);
mask(yrange(yi), xrange(xi), 2) = (ptn(yi, xi) == 1) .* color(2);
mask(yrange(yi), xrange(xi), 3) = (ptn(yi, xi) == 1) .* color(3);

im(mask > 0) = 0.3 .* im(mask > 0) + uint8(0.7 .* mask(mask > 0));

mask = any(mask > 0, 3);
[gx, gy] = gradient(double(mask));
outline = imdilate(repmat(conv2(gx.^2 + gy.^2, [1 1; 1 1], 'same') > 0, [1 1 3]), 1);

im(outline) = 255;

return;

end