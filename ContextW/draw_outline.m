function im = draw_outline(im, det, pattern, color)
det(1:4) = round(det(1:4));

mask = zeros(size(im));
    
h = det(4) - det(2) + 1;
w = det(3) - det(1) + 1;

ptn = imresize(pattern{det(5)}, [h w], 'nearest');

xrange = det(1):det(3);
yrange = det(2):det(4);

xi = find(xrange > 0 & xrange <= size(im, 2));
yi = find(yrange > 0 & yrange <= size(im, 1));

mask(yrange(yi), xrange(xi), 1) = ptn(yi, xi) >= 1;

[gx, gy] = gradient(double(mask));
outline = (gx.^2 + gy.^2) > 0;

for i=1:3
    temp = im(:,:,i);
    temp(outline) = color(i);
    im(:,:,i) = temp;
end


end