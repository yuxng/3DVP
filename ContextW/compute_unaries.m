function unaries = compute_unaries(dets, params)
unaries = zeros(size(dets, 1), 3);
for i = 1:size(dets, 1)
    unaries(i, 1) = dets(i, end) ./ params.snorm;
    unaries(i, 2) = params.b(dets(i, 5));
    unaries(i, 3) = measure_truncation_match(dets(i, 1:4), params.pattern{dets(i, 5)});
end
end