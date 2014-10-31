function prob = build_problem(unaries, pairwise, params)

% testing
% unaries(unaries(:, 1) < 0, 1) = unaries(unaries(:, 1) < 0, 1) .* 3;
% unaries(:, 1) = params.tw .* unaries(:, 1) + (1 - params.tw) .* unaries(:, 1) .^ 3;
% unaries(:, 1) = feat_transform(unaries(:, 1), params);
if(params.ver < 0.5)
    unaries(:) = feat_transform(unaries(:), params);

    prob.u = [unaries(:, 1), unaries(:, 2:3)] * params.w(1:3)' + params.bias;
    if(0)
        prob.p = pairwise(:,:,1) * params.w(4);
    elseif(0)
        prob.p = pairwise(:,:,1);
        prob.p(prob.p >= 0.5) = 1;
        prob.p(prob.p > 0.3 & prob.p < 0.5) = .2;
        prob.p(prob.p > 0.1 & prob.p < 0.3) = .1;
        prob.p(prob.p < 0.1) = 0;

        prob.p = prob.p .* params.w(4);
    else
        prob.p = feat_transform(pairwise(:,:,1), params) * params.w(4);
    end
    prob.p = prob.p + feat_transform(pairwise(:,:,2), params) * params.w(5);
else
    unaries(:, 1) = pwlinear(unaries(:, 1), params);
    prob.u = [unaries(:, 1), unaries(:, 2:3)] * params.w(1:3)' + params.bias;
    prob.p = pairwise(:,:,1) * params.w(4);
    prob.p = prob.p + pairwise(:,:,2) * params.w(5);
end

end


function feats = feat_transform(feats, params)
feats(:) = params.tw .* feats(:) + (1 - params.tw) .* feats(:) .^ 3;
end