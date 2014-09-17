
%create the feature vector for the true labelling of the instance

%% load 'trainval' image set
ids=textread(sprintf(VOCopts.imgsetpath,'trainval'),'%s');

% imgset='trainval';
% cls='bicycle';
% [ids,gt]=textread(sprintf(VOCopts.clsimgsetpath, cls,imgset),'%s %d');
 
 %considerIDS = [];
 %bic_ims= find(gt == 1);
 %for j = 1:numel(bic_ims)
 %    considerIDS = [considerIDS bic_ims(j)];
 %end

 considerIDS = 1:numel(ids);
%considerIDS =[1:3400 3500:3700];
 
numClasses=21;


%%%%
d_ij_size = 7;
for id = considerIDS
    load (strcat('CACHED_DATA_TRAINVAL/', ids{id}));

    Detections(:, 6) = 0;
    im=imread(sprintf(VOCopts.imgpath,ids{id}));
%     printBlob(I,rec.objects(1).bndbox, 'GTB', 1);
%     for ob=2:length(rec.objects)
%         printBlob(I,rec.objects(ob).bndbox, 'GTB', 0);
%     end
    % load the ground truth bounding boxes
    rec=PASreadrecord(sprintf(VOCopts.annopath,ids{id}));
    Detections = double(Detections);

    
    GT =[];
    %for every GT bounding box
    for gi=1:length(rec.objects)
        cls = rec.objects(gi).class;
        %get the class ID for this GT BB
        for clsID =1:numClasses
            if strcmp(VOCopts.classes{clsID}, rec.objects(gi).class)
                break
            end
        end
        

        clsDT = find(Detections(:, 5) == clsID & Detections(:, 6) ~= 1);
        n=length(clsDT);    
        
        x1 = double(Detections(clsDT,1));
        y1 = double(Detections(clsDT,2));
        x2 = double(Detections(clsDT,3));
        y2 = double(Detections(clsDT,4));
        ba = (x2-x1+1) .* (y2-y1+1);

        %pick the one with the highest overlap
        gbox = rec.objects(gi).bbox;
        gx1 = gbox(1);
        gy1 = gbox(2);
        gx2 = gbox(3);
        gy2 = gbox(4);
        ga  = (gx2-gx1+1) .* (gy2-gy1+1);

        xx1 = max(x1, gx1);
        yy1 = max(y1, gy1);
        xx2 = min(x2, gx2);
        yy2 = min(y2, gy2);

        w = xx2-xx1+1;
        h = yy2-yy1+1;
        I = find(w > 0 & h > 0);
        int   = w(I).*h(I);
        ov    = zeros(n,1);
        ov(I) = int ./ (ba(I) + ga - int);

        %ov(find(ov < 0.5)) = 0;
        
        [v,j] = max(ov);
        
        %label as true +ve the detection that overlaps the GT most
        if v > 0.5
            Detections(clsDT(j), 6) =1;
        end
        

             
    end

    numCl = 21;
    PSI_true = zeros(numCl^2*d_ij_size, 1);
    PHI_true = zeros(numCl*2, 1);
    non_bg = find(Detections(:, 6) == 1);


    
    [PSI_true PHI_true] = computeFeature(double(Detections(non_bg, :)), double(Scores(non_bg)));
  
    Feat_true = [PSI_true; PHI_true];
  
    
    
    if mod(id, 20) == 0
        id
        datestr(now)
    end
    save (strcat('FEAT_TRUE_TRAINVAL/', ids{id}), 'Feat_true');
end