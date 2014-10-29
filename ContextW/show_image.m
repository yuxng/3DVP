function im = show_image(indices, i)

setpath;

imfile  = sprintf('%s/%06d.png', image_dir, indices(i));
im = imread(imfile);
if(nargout == 1)
else
    imshow(im);
end
end