for id = considerIDS
    if exist(strcat('CACHED_DATA_TRAINVAL/', ids{id}, '.mat')) >0 
        rec=PASreadrecord(sprintf(VOCopts.annopath,ids{id}));
        GT =[];
        for gi=1:length(rec.objects)
            cls = rec.objects(gi).class;
            for clsID =1:numClasses
                if strcmp(VOCopts.classes{clsID}, rec.objects(gi).class)
                    break
                end
            end
            GT = [GT ;rec.objects(gi).bbox clsID];
        end
        load(strcat('CACHED_DATA_TRAINVAL/', ids{id}));
               
        nDet = size(Detections, 1);
    
        ptr = nDet;
        while length(find(Detections(ptr, :)>0)) == 0
            ptr = ptr-1;
        end
        nDet = ptr;
        
        loss = computeloss(Detections(1:nDet, 1:5), GT);
        
        save(strcat('LOSS_TRAINVAL/', ids{id}), 'loss');
    end
    if mod(id, 100) ==0
        id
        datestr(now)
    end
end


