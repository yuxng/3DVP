function [I, S] = find_MVC_test(W_s, W_a, centers, Tdata)

% load detections
Detections = Tdata.Detections;
Scores = Tdata.Scores;
Matching = Tdata.Matching;
Overlaps = Tdata.Overlaps;
Idx = Tdata.Idx;
Matching(Overlaps < 0.1) = 1;

% cluster of detections
cdets = unique(Idx);
cdets(cdets == -1) = [];
num = numel(cdets);

% Initial energy is just the weighted local scores 
E = zeros(num, 1);
for i = 1:numel(centers)
    index = find(Detections(cdets, 5) == centers(i));
    if isempty(index) == 0
        E(index) = W_a(2*i - 1) .* Scores(cdets(index)) + W_a(2*i);
    end
end

Pos = E;
Neg = zeros(num,1);

[I, S] = maximize(Matching, Idx, Pos, Neg, W_s);
