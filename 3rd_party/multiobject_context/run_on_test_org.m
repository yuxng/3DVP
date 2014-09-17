current = pwd;
vocdevkit_root = '/home/chaitanya/VOCdevkit/';
addpath (vocdevkit_root);
addpath(strcat(vocdevkit_root, '/VOCcode'));
cd (vocdevkit_root);
VOCinit;

cd (current);


numClasses = length(VOCopts.classes);
CLASSES = [1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20];

global ids d_ij_size Detections Scores;


% load 'test' image set
ids=textread(sprintf(VOCopts.imgsetpath,'test'),'%s');

numClasses = numClasses+1;

considerIDS = 1:length(ids) ;
%considerIDS=[1:3400 3500:3700];
d_ij_size = 7;


%create files tow h
for clsID=1:20
    cls = VOCopts.classes{clsID};
    fid_context{clsID} = fopen(sprintf(VOCopts.detrespath,'org' ,cls),'w');
end

for i_id=considerIDS
    rec=PASreadrecord(sprintf(VOCopts.annopath,ids{i_id}));
    I=imread(sprintf(VOCopts.imgpath,ids{i_id}));
    %load the precmputed cached variables Detections and Scores
    %that represent the bounding boxes and the unary scores for this
    %test image respectively.

    % Detections(i, :) = [x1 y1 x2 y2 clsID]
    % S(i) = score of the local model on the ith detection

    load(strcat('CACHED_DATA_TEST_CD/', ids{i_id}));
    if(mod(i_id, 100) == 0)
        i_id
    end

    %[E H_wo_gr]  = find_MVC_test(W_s, W_a, numClasses);

    %TP = find(H_wo_gr == 1);

    %nDet = size(Detections, 1);

    %ptr = nDet;
    %while length(find(Detections(ptr, :)>0)) == 0
    %    ptr = ptr-1;
    %end
    %nDet = ptr;

    for d=1:size(Detections,1)
        fprintf(fid_context{Detections(d, 5)}, '%s %f %f %f %f %f\n',ids{i_id}, Scores(d),Detections(d, 1:4));
    end

end

AP_B = 0;
AP_GNM = 0;
AP_M2 = 0;
precision=[];
recall=[];
ap=[];

for clsID=1:20
    cls=VOCopts.classes{clsID};
    figure

    [recall precision ap(clsID)]=VOCevaldet( VOCopts,'org',cls,true);  % compute and display PR

    saveas(gcf, strcat('results/testCD/', cls, 'org', '.jpg'));
    close(gcf);
end
meanap = mean(ap);
save ('ap_org', 'meanap');


