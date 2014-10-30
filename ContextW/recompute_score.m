function dets = recompute_score(dets, solution, unaries, pairwise)

scores = zeros(size(dets, 1), 1);
scores(:) = unaries(:) + sum(pairwise(:, solution), 2);
dets(:, end) = scores;

end