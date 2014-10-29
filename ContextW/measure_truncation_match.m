function out = measure_truncation_match(box, pattern, imsize)
% the function take the full extent of the box as an input! 

if nargin < 3
    imsize = [1224, 370]; % kittisize
end

objarea = sum(pattern(:) >= 1);

rt = box2rect(box);

xstep = rt(3) / size(pattern, 2);
ystep = rt(4) / size(pattern, 1);

boxx = repmat(box(1):xstep:box(3), size(pattern, 1), 1);
boxy = repmat((box(2):ystep:box(4))', 1, size(pattern, 2));

assert(size(boxx, 2) >= size(pattern, 2));
assert(size(boxy, 1) >= size(pattern, 1));

boxx = boxx(:, 1:size(pattern, 2));
boxy = boxy(1:size(pattern, 1), :);

inimage = boxx >= 0 & boxx <= imsize(1) & boxy >= 0 & boxy <= imsize(2);
pattern(inimage & pattern == 3) = 1;
matchedarea = sum(pattern(:) >= 3);

out = matchedarea ./ objarea;

end