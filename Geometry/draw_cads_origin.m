function draw_cads_origin(cls)

% load CAD models
filename = sprintf('%s_kitti.mat', cls);
object = load(filename);
cads = object.(cls);

N = numel(cads);
for i = 1:N
    cad = cads(i);
    
    subplot(3,4,i)
    trimesh(cad.faces, cad.vertices(:,1), cad.vertices(:,2), cad.vertices(:,3), 'EdgeColor', 'b');
    axis equal;
    axis off;
end