% prepare the training data to train the contextual model
function prepare_training_data_add_idx

nms_threshold = 0.6;
cache_dir = 'CACHED_DATA_TRAINVAL';

% load ids
object = load('kitti_ids_new.mat');
ids = [object.ids_train object.ids_val];

N = numel(ids);
for i = 1:N
    filename = fullfile(cache_dir, sprintf('%06d.mat', ids(i)));
    disp(filename);
    object = load(filename);
    Detections = object.Detections;
    Scores = object.Scores;
    Patterns = object.Patterns;
    Overlaps = object.Overlaps;
    Matching = object.Matching;
    det = [Detections Scores];
    Idx = nms_clustering(det, nms_threshold);    
    parsave(filename, Detections, Scores, Patterns, Overlaps, Matching, Idx);
end


function parsave(filename, Detections, Scores, Patterns, Overlaps, Matching, Idx)

save(filename, 'Detections', 'Scores', 'Patterns', 'Overlaps', 'Matching', 'Idx', '-v7.3');