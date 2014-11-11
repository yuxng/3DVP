function exemplar_3drr_test_combined

matlabpool open;

is_save = 1;

cls = 'car';
result_dir = 'kitti_test_acf_3d_227_flip';
name = '3d_ap_227';

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

% 3DRR path
image_dir = '/home/yuxiang/Projects/ObjectInteraction/dataset_3dRR13/Images/car'; 

% get test image ids
ids = 1:200;

% run detector in each image
N = numel(ids);
boxes = cell(1, N);
parfor id = 1:N
    tic;
    fprintf('%s %s: combined: %d/%d\n', cls, name, id, N);
    file_img = sprintf('%s/%04d.jpg', image_dir, ids(id));
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
    
    if is_save == 0
        imshow(I);
        hold on;
        flag = nms(bbs, 0.5);
        det = bbs(flag, :);
        threshold = -20;
        
        for k = 1:size(det,1)
            if det(k,6) > threshold
                % get predicted bounding box
                bbox_pr = det(k,1:4);
                bbox_draw = [bbox_pr(1), bbox_pr(2), bbox_pr(3)-bbox_pr(1), bbox_pr(4)-bbox_pr(2)];
                rectangle('Position', bbox_draw, 'EdgeColor', 'g', 'LineWidth', 2);
            end
        end
        hold off;
        
        pause;
    end
end

if is_save
    dets = boxes;
    filename = fullfile(result_dir, sprintf('%s_%s_combined_3drr.mat', cls, name));
    save(filename, 'dets', '-v7.3');
end

matlabpool close;