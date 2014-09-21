% example usage: main(2)
% paramemeter C := tradeoff between hinge loss and regularizer
function main(C)

is_train = 1;

% load data
if is_train
    object = load('../KITTI/data.mat');
else
    object = load('../KITTI/data_all.mat');
end
data = object.data;
centers = unique(data.idx_ap);
numClasses = numel(centers);

% load ids
object = load('kitti_ids.mat');
if is_train
    ids = object.ids_train;
else
    ids = [object.ids_train object.ids_val];
end

% load training data
fprintf('load training data...');
Tdata = [];
for i = 1:numel(ids)
    id = ids(i);
    % load groundtruth feature
    filename = fullfile('FEAT_TRUE_TRAINVAL', sprintf('%04d.mat', id));
    object = load(filename);
    Tdata(i).Feat_true = object.Feat_true;

    % load detections
    filename = fullfile('CACHED_DATA_TRAINVAL', sprintf('%04d.mat', id));
    object = load(filename);
    Tdata(i).Detections = object.Detections;
    Tdata(i).Scores = object.Scores;
    Tdata(i).Matching = object.Matching;

    % load loss
    filename = fullfile('LOSS_TRAINVAL', sprintf('%04d.mat', id));
    object = load(filename);
    Tdata(i).loss = object.loss;
end
fprintf('done\n');
   
% initialize the weights
W_s = rand(2, 1);
W_a = rand(numClasses*2, 1);

MAX_CON = 100000;
Constraints = zeros(length(W_s) + length(W_a), MAX_CON, 'single');
Margins = zeros(1, MAX_CON, 'single');
IDS = zeros(1, MAX_CON, 'single');
Labelings  = zeros(500, MAX_CON, 'single'); % assume no image contains more than 500 detections across all classes

max_iter = 500;
iter = 1;
trigger = 1;
low_bound = 0;
n = 0;

w = [W_s; W_a];
cost = w'*w*.5;

while (iter < max_iter && trigger)
    datestr(now);
    trigger = 0;

    for id = 1:numel(ids)
        [H_wo, X_wo, m]  = find_MVC(W_s, W_a, centers, Tdata(id));
        
        % if this constraint is the MVC for this image
        isMVC = 1;
        check_labels = find(IDS(1, 1:n) == id);
        score = m - w'*X_wo;
        
        for ii = 1:numel(check_labels)
            label_ii = check_labels(ii);
            if m - w'*Constraints(:, label_ii) > score
                isMVC = 0;
                break;
            end
        end
       
        if isMVC ==1
            cost = cost + C * max(0, m - w'*X_wo);
            % add only if this is a hard constraint
            if (m - w'*X_wo) >= -0.001
                n = n + 1;
                Constraints(:, n) = X_wo;
                Margins(n) = m;
                IDS(n) = id;

                if n > MAX_CON
                    disp('n > MAX_CON');
                    [slacks, I_ids] = sort((Margins(:,n)  - w'*Constraints(:, 1:n)), 'descend');
                    J = I_ids(1:MAX_CON);
                    n = length(J);
                    Constraints(:, 1:n) = Constraints(:, J);
                    Margins(:, 1:n) = Margins(:, J);
                    IDS(:, 1:n) = IDS(:, J);
                end
            end
        end

        if 1 - low_bound/cost > 0.01
            % Call QP
            [w, cache] = lsvmopt_new(Constraints(:, 1:n), Margins(1:n), IDS(1:n), C, 0.01, []);

            % Prune working set
            I = find(cache.sv > 0);
           
            n = length(I);
            Constraints(:, 1:n) = Constraints(:, I);

            Margins(:, 1:n) = Margins(:, I);
            IDS(:, 1:n) = IDS(:, I);
            Labelings(:, 1:n) = Labelings(:, I);
           
            % Update parameters
            W_s = w(1:length(W_s));
            W_a = w(length(W_s)+1:end);
            
            %reset the running estimate on upper bund
            cost = w'*w*0.5;
            low_bound = cache.lb;
            trigger = 1;
        end
    end

    iter = iter +1;
    if trigger == 0
        fprintf('not triggered\n');
    end
    
    save('wts_trainval_pascal', 'w');
end
disp('converged');