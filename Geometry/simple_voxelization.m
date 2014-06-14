function grid = simple_voxelization(vertices, N)

grid = zeros(N, N, N);

for i = 1:size(vertices,1)
    index = floor((vertices(i,:) + [0.5 0.5 0.5]) * N) + 1;
    index(index > N) = N;
    grid(index(1), index(2), index(3)) = 1;
end