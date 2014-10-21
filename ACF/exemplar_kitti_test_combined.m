function exemplar_kitti_test_combined

matlabpool open;

cls = 'car';
is_train = 1;
result_dir = 'kitti_train_ap_125';
name = '3d_aps_125';

exemplar_globals;

% load detector
model_name = fullfile(result_dir, sprintf('%s_%s_combined_detector.mat', cls, name));
object = load(model_name);
Ds = object.detectors;

% set up
nDs = length(Ds);
opts = Ds{1}.opts;
pPyramid = opts.pPyramid;
pNms = opts.pNms;
imreadf = opts.imreadf;
imreadp = opts.imreadp;
shrink = pPyramid.pChns.shrink;
pad = pPyramid.pad;
separate = nDs > 1 && isfield(pNms, 'separate') && pNms.separate;

% KITTI path
root_dir = KITTIroot;
if is_train == 1
    data_set = 'training';
else
    data_set = 'testing';
end
cam = 2;
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]); 

% get test image ids
filename = fullfile(SLMroot, 'ACF/kitti_ids_new.mat');
object = load(filename);
if is_train == 1
    % ids = object.ids_val;
    ids = object.ids_train;
else
    ids = object.ids_test;
end

filename = fullfile(result_dir, sprintf('%s_%s_combined_train.mat', cls, name));

% run detector in each image
N = numel(ids);
boxes = cell(1, N);
parfor id = 1:N
    tic;
    fprintf('%s %s: combined: %d/%d\n', cls, name, id, N);
    file_img = sprintf('%s/%06d.png', image_dir, ids(id));
    I = feval(imreadf, file_img, imreadp{:});

    P = chnsPyramid(I, pPyramid);
    bbs = cell(P.nScales, nDs);
    for i = 1:P.nScales
        for j = 1:nDs
            opts = Ds{j}.opts;
            modelDsPad = opts.modelDsPad;
            modelDs = opts.modelDs;
            bb = acfDetect1(P.data{i}, Ds{j}.clf, shrink,...
              modelDsPad(1), modelDsPad(2), opts.stride, opts.cascThr);
            shift = (modelDsPad - modelDs) / 2 - pad;
            bb(:,1) = (bb(:,1)+shift(2)) / P.scaleshw(i,2);
            bb(:,2) = (bb(:,2)+shift(1)) / P.scaleshw(i,1);
            bb(:,3) = modelDs(2) / P.scales(i);
            bb(:,4) = modelDs(1) / P.scales(i);
            if separate
                bb(:,6) = j;
            end
            num = size(bb,1);
            bb = [bb(:,1) bb(:,2) bb(:,1)+bb(:,3) bb(:,2)+bb(:,4) Ds{j}.cid*ones(num,1) bb(:,5)];
            bbs{i,j} = bb;
        end
    end
    bbs = cat(1, bbs{:});
    boxes{id} = bbs;
    toc;
    % no non-maximum suppression
end
dets = boxes;
save(filename, 'dets', '-v7.3');

matlabpool close;