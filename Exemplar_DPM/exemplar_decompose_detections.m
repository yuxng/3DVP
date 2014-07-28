function exemplar_decompose_detections(dets_3d)

N = numel(dets_3d);

for i = 1:N
    objects = dets_3d{i};
    filename = sprintf('results/%06d_3d.mat', i-1);
    disp(filename);
    save(filename, 'objects');
end