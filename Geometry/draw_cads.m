function draw_cads(cls)

% load CAD models
filename = sprintf('%s.mat', cls);
object = load(filename);
cads = object.(cls);

N = numel(cads);
for i = 1:N
    cad = cads(i);

    L3 = cad.grid;
    L3(L3 == 1) = 127;
    L3(L3 == 0) = 1;
    
    subplot(3,4,i)
    drawLabeledVoxel(L3);
end