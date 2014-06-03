% find the closest CAD model
function cad_index = find_closest_cad(cads, object)

r = [object.l/object.h object.l/object.w object.h/object.w];

% rotation matrix to transform coordinate systems
Rx = [1 0 0; 0 0 -1; 0 1 0];
Ry = [cos(-pi/2) 0 sin(-pi/2); 0 1 0; -sin(-pi/2) 0 cos(-pi/2)];
num = numel(cads);
dis = zeros(num, 1);

for i = 1:num
    cad = cads(i);
    x3d = Ry*Rx*cad.vertices';
    
    l = max(x3d(1,:)) - min(x3d(1,:));
    h = max(x3d(2,:)) - min(x3d(2,:));
    w = max(x3d(3,:)) - min(x3d(3,:));
    
    ratio = [l/h l/w h/w];
    dis(i) = norm(ratio - r);
end

[~, cad_index] = min(dis);