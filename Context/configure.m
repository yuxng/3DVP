function configure

mex addcols.cc
mex qp.cc
mex maximize.cc

% extract ground truth feature vector from fixing the true +ves
extract_groundtruth_features;
fprintf('\n\n..done extracting feature vectors from the fxed true +ves \n\n');

%pre-compute the loss for turning on/off each detection based on the fixed
%true +ves
precompute_loss_values;
fprintf('\n\n..done setting on/off loss for different detections \n\n');