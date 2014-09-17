
mex addcols.cc
mex qp.cc

mex computeFeature.cc
mex maximize.cc

current = pwd;

% add path to VOCDevkit
vocdevkit_root = '/home/chaitanya/VOCdevkit/';
addpath (vocdevkit_root);
addpath(strcat(vocdevkit_root, '/VOCcode'));

cd (vocdevkit_root);
VOCinit;

cd (current);

%extract ground truth feature vector from fixing the true +ves
extract_feat_TP_1
fprintf('\n\n..done extracting feature vectors from the fxed true +ves \n\n');

%pre-compute the loss for turning on/off each detection based on the fixed
%true +ves
loss_full
fprintf('\n\n..done setting on/off loss for different detections \n\n');
