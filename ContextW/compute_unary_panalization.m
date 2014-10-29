function [b] = compute_unary_panalization(patterns)

b = zeros(length(patterns), 1);

for i = 1:length(patterns)
    objarea = sum(patterns{i}(:) >= 1);
    occarea = sum(patterns{i}(:) >= 2);
    b(i) = occarea ./ objarea;
end

end