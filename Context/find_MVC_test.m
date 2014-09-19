function [I, S] = find_MVC_test(W_s, W_a, centers, Tdata)

% load detections
Detections = Tdata.Detections;
Scores = Tdata.Scores;
Matching = Tdata.Matching;

num = size(Detections, 1);
E = zeros(num, 1);
for i = 1:numel(centers)
    index = find(Detections(:, 5) == centers(i));
    if isempty(index) == 0
        E(index) = W_a(2*i - 1) .* Scores(index) + W_a(2*i);
    end
end

Pos = E;
Neg = zeros(num,1);

[I, S] = maximize(Detections, Matching, Pos, Neg, W_s);
