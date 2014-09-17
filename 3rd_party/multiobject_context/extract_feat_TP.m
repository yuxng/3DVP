
%create the ground truth feature vector for each of the training instances
%by 
% i) summing up the local features (in this case simply the scores of the
%local detectors) across true +ve detections and 
% ii) summing up the pairwise spatial relationships across true +ve
% detections
% iii) concatenating i) and ii)


%% load 'training' data
ids=textread(sprintf(VOCopts.imgsetpath,'trainval'),'%s');


considerIDS = 1:numel(ids);

% set the number of classes in your application
% in this case, it is number of classes for PASCAL'07 + background = 21
  
numClasses=21;

% size of the sptial bin defining relationship between windows i and j
d_ij_size = 7;

for id = considerIDS
    load (strcat('CACHED_DATA_TRAINVAL/', ids{id}));

    Detections(:, 6) = 0;
    I=imread(sprintf(VOCopts.imgpath,ids{id}));
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

    % set the number of classes in your application
    % in this case, it is number of classes for PASCAL'07 + background = 21
  
    numClasses = 21;
    PSI_true = zeros(numClasses^2*d_ij_size, 1);
    PHI_true = zeros(numClasses*2, 1);
    non_bg = find(Detections(:, 6) == 1);

    for i=1:numel(non_bg)
        d = non_bg(i);
       
        y_d = Detections(d, 5);
        bbox_i = Detections(d, 1:4);
        height_i = bbox_i(4)- bbox_i(2) + 1;
        width_i = bbox_i(3)- bbox_i(1) +1 ;
        i_cent_x = bbox_i(1) + width_i/2;
        i_cent_y =  bbox_i(2) + height_i/2;
    
        
         Selected = [];
        Selected.x = [];
        Selected.y = [];
        Selected.w = [];
        Selected.h = [];
        Selected.x = bbox_i(1);
        Selected.y = bbox_i(2);
       Selected.h = bbox_i(4)- bbox_i(2);
       Selected.w = bbox_i(3) - bbox_i(1);
        %printBlob(I,Selected, 'SRT', 0);
        
        for j=1:numel(non_bg)
            p = non_bg(j);
        
            if d ~= p
                psi_pair = zeros(numClasses^2*d_ij_size, 1);

                y_p = Detections(p,5);
                bbox_j = Detections(p,1:4);

                Selected = [];
                Selected.x = [];
                Selected.y = [];
                Selected.w = [];
                Selected.h = [];
               Selected.x = bbox_j(1);
                Selected.y = bbox_j(2);
               Selected.h = bbox_j(4)- bbox_j(2);
                Selected.w = bbox_j(3) - bbox_j(1);
                %printBlob(I,Selected, 'SRT', 0);
                
                
                bi=[max(bbox_i(1),bbox_j(1)) ; max(bbox_i(2),bbox_j(2)) ; min(bbox_i(3),bbox_j(3)) ; min(bbox_i(4),bbox_j(4))];
                iw=bi(3)-bi(1)+1;
                ih=bi(4)-bi(2)+1;
                ov=0;
                if iw>0 & ih>0
                    % compute overlap as area of intersection / area of union
                    ua=(bbox_i(3)-bbox_i(1)+1)*(bbox_i(4)-bbox_i(2)+1)+...
                        (bbox_j(3)-bbox_j(1)+1)*(bbox_j(4)-bbox_j(2)+1)-...
                        iw*ih;
                    ov=iw*ih/ua;
                end
                
                j_cent_x = bbox_j(1) + ( bbox_j(3)- bbox_j(1))/2;
                j_cent_y =  bbox_j(2) + ( bbox_j(4)- bbox_j(2))/2;
                
                      
                %%%
                %% compute the spatial feature of p w.r.t d
                %%%
                spatial = zeros(d_ij_size,1);
                
                %on top of
                if(abs(i_cent_x - j_cent_x)<= width_i/2 &...
                        abs(i_cent_y - j_cent_y)<= height_i/2)
                    spatial(1) = 1;
                end

                %above
                if(j_cent_y < (i_cent_y - height_i/2) & ...
                        j_cent_y >= (i_cent_y - 1.5*height_i) &...
                        abs(i_cent_x - j_cent_x) <= width_i/2)
                    spatial(2) = 1;
                end

                %below
                if (j_cent_y > (i_cent_y + height_i/2) & ...
                        j_cent_y <= (i_cent_y + 1.5*height_i) &...
                        abs(i_cent_x - j_cent_x) <= width_i/2)
                    spatial(3) = 1;
                end

                %next to
                if (abs(i_cent_y - j_cent_y) <= height_i/2 & ...
                        abs(i_cent_x - j_cent_x) > width_i/2 &...
                        abs(i_cent_x - j_cent_x) <= 1.5*width_i)
                    spatial(4) = 1;
                end

                %near relationship
                if (abs(i_cent_x - j_cent_x) > width_i/2 &...
                       abs(i_cent_x - j_cent_x) <= 1.5*width_i &...
                       abs(i_cent_y - j_cent_y) > height_i/2 &...
                       abs(i_cent_y -j_cent_y) <= 1.5*height_i)
                   spatial(5)=1;
                end

                %far
                if (abs(i_cent_x - j_cent_x) > 1.5*width_i | ...
                        abs(i_cent_y - j_cent_y) > 1.5*height_i)

                   spatial(6) = 1;
                else %near
                   spatial(5) = 1;
                end
                
                if ov > .5
                    spatial(7) =1;
                end
                  
                start = (y_d - 1)*numClasses*d_ij_size + (y_p-1)*d_ij_size +1;
                stop = start + d_ij_size -1;
                psi_pair(start:stop) = double(spatial);
                PSI_true = PSI_true + psi_pair;
            end
        end
        phi_loc = zeros(numClasses*2, 1);
        if y_d ~= 21
            %phi_loc(2*y_d-1) = Detections(d, 2);
            phi_loc(2*y_d-1) = Scores(d);
            phi_loc(2*y_d) = 1;
        end
        PHI_true = PHI_true + phi_loc;
    end
    Feat_true = [PSI_true; PHI_true];
    [PSI_wo_mex PHI_wo_mex] = computeFeature_desaic(double(Detections(non_bg, :)), double(Scores(non_bg)));

    if  ~isequal(PSI_wo_mex,PSI_true);
        keyboard
    end
    if  ~isequal(PHI_wo_mex,PHI_true)
        keyboard
    end
    if mod(id, 20) == 0
        id
        datestr(now)
    end
    save (strcat('FEAT_TRUE_TRAINVAL/', ids{id}), 'Feat_true');
end