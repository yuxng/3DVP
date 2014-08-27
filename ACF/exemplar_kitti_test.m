function exemplar_kitti_test(cls, cid, is_train, is_continue)

exemplar_globals;

% load detector
model_name = fullfile(resultdir, sprintf('%s_%d_final.mat', cls, cid));
object = load(model_name);
detector = object.detector;
detector = acfModify(detector, 'cascThr', -20);

Ds = detector;
if ~iscell(Ds)
    Ds = {Ds};
end

nDs = length(Ds);
opts = Ds{1}.opts;
pPyramid = opts.pPyramid;
pNms = opts.pNms;
imreadf = opts.imreadf;
imreadp = opts.imreadp;
shrink = pPyramid.pChns.shrink;
pad = pPyramid.pad;
separate = nDs > 1 && isfield(pNms, 'separate') && pNms.separate;
is_hadoop = opts.is_hadoop;

% KITTI path
root_dir = KITTIroot;
if is_train == 1
    data_set = 'training';
else
    data_set = 'testing';
end

% get sub-directories
cam = 2; % 2 = left color camera
image_dir = fullfile(root_dir, [data_set '/image_' num2str(cam)]); 

% get test image ids
filename = fullfile(SLMroot, 'ACF/kitti_ids.mat');
object = load(filename);
if is_train == 1
    ids = [object.ids_train object.ids_val];
else
    ids = object.ids_test;
end

filename = fullfile(resultdir, sprintf('%s_%d_test.mat', cls, cid));

% run detector in each image
if is_continue == 1 && exist(filename, 'file')
    load(filename);
else
    N = numel(ids);
    boxes = cell(1, N);
    parfor id = 1:N
        fprintf('%s: center %d: %d/%d\n', cls, cid, id, N);
        img_idx = ids(id);
        file_img = sprintf('%s/%06d.png', image_dir, img_idx);
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
                bbs{i,j} = bb;
            end
        end
        bbs = cat(1, bbs{:});
        boxes{id} = bbs;
        % no non-maximum suppression
        if is_hadoop
            tempstring = sprintf('%d objects detected in image %d', size(bbs,1), img_idx);
            stdout_withFlush(tempstring);        
        end
    end  
    save(filename, 'boxes', '-v7.3');
end