% ========================================
% drawLabeledVoxel.m 
% ========================================
function drawLabeledVoxel(L3, v1, v2)

% L3 is [nx x ny x nz] 3d dimension.
% Range(L3)=1~N_CLASS, where 1 indicate empty space

if isempty(find(L3>1, 1)),
    return;
end

tL3  = L3 - 1;
tL3(tL3==0) = NaN;
colormap('default');
% cmap = colormap;
cmap = [0 0 1; 0 1 0; 1 0 0; 0 1 1];
% cmap = colormap(summer);
PATCH_3Darray(tL3+1, cmap, 'col');

xlim([0 size(L3,1)]);
ylim([0 size(L3,2)]);
zlim([0 size(L3,3)]);

if nargin < 4,
    view(330,20);
else
    view(v1,v2);
end
axis equal;
axis tight;
% ========================================