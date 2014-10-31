function show_results(imageidx, dets, params)

load ../KITTI/kitti_ids_new.mat

im = show_image(ids_val, imageidx);
imshow(im);
% cols = colormap;
cols = [1, 0, 0; 0, 1, 0; 0, 0, 1; 1, 1, 0; 0, 1, 1; 1, 0, 1; 0, 0, 0; 1, 1, 1;];
for i = 1:size(dets, 1)
    col = cols(mod(i*1, size(cols, 1))+1, :);
    im = draw_object(im, dets(i, :), params.pattern, col .* 255);
end
imshow(im);

for i = 1:size(dets, 1)
    col = cols(mod(i*1, size(cols, 1))+1, :);
    rectangle('position', box2rect(dets(i, 1:4)), 'linewidth', 2, 'edgecolor', col);
end
drawnow;

end