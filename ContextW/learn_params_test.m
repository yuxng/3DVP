function params = learn_params_test(data)

% load the training parameters
object = load('training_params.mat');
params = object.params;

centers = unique(data.idx_ap);
centers(centers == -1) = [];

% compute pattern penalization cost
b = sparse(length(data.pattern), 1);
b(centers) = compute_unary_panalization(data.pattern(centers));

% get the box transforms
transform = zeros(length(data.pattern), 4);
for i = 1:length(centers)
    if(any(data.pattern{centers(i)}(:) == 3))
        dummybox = [1 1 100 100]; w = 100; h = 100;
        bboxnew = exemplar_transform_truncated_box(dummybox, data.pattern{centers(i)});
        
        transform(centers(i), :) = (bboxnew - dummybox) ./ [w h w h];
    end
end

params.b = b;
params.transform = transform;
params.centers = centers;
params.pattern = data.pattern;