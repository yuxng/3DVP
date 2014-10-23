% inference
% /*
%  *  maximize.cc
%  *  [I,S] = maximize(Detections,PosScore,NegScore,Weights)
%  *
%  *  Inputs:
%  *
%  *  Detections: N X D matrix where D(i,:) = [x1 y1 x2 y2 class]
%  *  PosScore: N x 1 matrix   where P(i)   = local score for turning on box i
%  *  NegScore: N x 1 matrix   where N(i)   = local score for turning off box i
%  *  Weights:  Vector of length Numclass*numClass*NumSpatFeatures
%  *
%  *  Outputs:
%  *
%  *  I: N by 1 matrix where I(i) = 1 if box i is turned on
%  *  S: N by 1 matrix where S(i) = accumulated score of box i
%  */
function [I, S] = maximize(Matching, Idx, PosScore, NegScore, Weights)

cdets = unique(Idx);
cdets(cdets == -1) = [];
N = numel(cdets);

% initialize instance set "I" to all 0s and "S" to pos
I = zeros(N, 1);
S = PosScore;

num_pos = 0;
while num_pos < N
    % Find highest scoring un-instanced box "i" and 
    % make sure it's positive score is better than negative score    
    index = find(I == 0);
    [smax, ind] = max(S(index));
    ind = index(ind);
    
    if smax < NegScore(ind)
        break;
    end
    
    % turn on detection
    I(ind) = 1;
    num_pos = num_pos + 1;
    
    % update S
    for i = 1:N
        if I(i) == 1
            continue;
        end
        
        % get the matching score
        s = max(max(Matching(Idx == cdets(ind), Idx == cdets(i))));     
        
        S(i) = S(i) + Weights(1) * s + Weights(2);
    end
end