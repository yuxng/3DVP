function draw_cad(cad, visibility)

if nargin < 2
    visibility = [];
end

L3 = cad.grid;
if isempty(visibility) == 0
    L3(L3 == 1) = 2;  % self-occluded
else
    L3(L3 == 1) = 3;
end
L3(L3 == 0) = 1;  % empty voxel

if isempty(visibility) == 0
    L3(visibility == 1) = 3;  % visiblie
    L3(visibility == 2) = 4;  % occluded by other objects
    L3(visibility == 3) = 5;  % truncated
end
drawLabeledVoxel(L3);