function y = pwlinear(x, lambda)
%  y = pwlinear(0:0.01:1, [0 0.1 0.9 1; 0 5 0 0; 0 -0.5 4 0]);

y = x;
for i = 1:size(lambda, 2)-1
    t = x(:) >= lambda(1, i) & x(:) <= lambda(1, i + 1);
    y(t) = lambda(2, i) * x(t) + lambda(3, i);
end

end
