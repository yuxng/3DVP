function [H_wo X_wo margin] = find_MVC(W_s, W_a, numClasses, i_id)
    global ids;
    
    
    load (strcat('FEAT_TRUE_TRAINVAL/', ids{i_id}));
    Feat_true = double(Feat_true);

    load(strcat('CACHED_DATA_TRAINVAL/', ids{i_id}));
    Detections= double(Detections);
    Scores = double(Scores);

    load(strcat('LOSS_TRAINVAL/', ids{i_id}));
    loss  = double(loss);
    nDet = size(Detections, 1);
    
    ptr = nDet;
    while length(find(Detections(ptr, :)>0)) == 0
        ptr = ptr-1;
    end
    nDet = ptr;
    
     %Initial energy is just the weighted local scores 
    E = zeros(nDet,1);
    for clsID = 1:numClasses-1
        cls_dets = find(Detections(:, 5) == clsID);
        E(cls_dets) = W_a(2*clsID - 1).*Scores(cls_dets) + W_a(2*clsID);
    end
    
    Pos = E + double(loss(:,1));
    Neg  = double(loss(:, 2));
    
    

    [E_mex H_wo] = maximize(double(Detections(1:nDet, :)),Pos,Neg,W_s);
    inds = find(H_wo == 1);
    [PSI_wo_mex PHI_wo_mex] = computeFeature(double(Detections(inds, :)), double(Scores(inds)));
    Feature_wo = [PSI_wo_mex; PHI_wo_mex];
    
    X_wo = Feat_true -  Feature_wo;
    bg = find(H_wo ==0);
    I = find(H_wo == 1);
    margin = sum(loss(I,1)) + sum(loss(bg, 2));
    %mvc_score = [W_s; W_a]'*[PSI_wo_mex; PHI_wo_mex] + margin;
end
    
