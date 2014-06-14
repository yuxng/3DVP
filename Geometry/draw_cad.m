function draw_cad(cad, visibility)

if nargin < 2
    visibility = [];
end

L3 = cad.grid;
if isempty(visibility) == 0
    L3(L3 == 1) = 2;
else
    L3(L3 == 1) = 127;
end
L3(L3 == 0) = 1;

if isempty(visibility) == 0
    L3(visibility == 1) = 127;
end
drawLabeledVoxel(L3);