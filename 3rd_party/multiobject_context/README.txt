This package is an implementation of the structured prediction approach for learning a multi class object detection model that predicts a structured output as described in
( Desai C., Ramanan D., Fowlkes C. "Discriminative models for multi-class object layout", ICCV 2009).
It uses cutting plane optimization to learn a contextual model based on spatial co-occurrence statistics of objects in images.  See the paper for a more detailed description of the model. 

Pre-Processing 
---------------------
1. Caching Bounding Boxes by running local detectors: 
Before training the contextual model, the code expects cached bounding boxes from running local detectors on the training set to reside in a folder named "CACHED_DATA_TRAINVAL".  CACHED_DATA_TRAINVAL  should reside in the same folder in which you extracted this folder, "multiobject_context". The name of each .mat file inside CACHED_DATA_TRAINVAL folder will be an image id in your training set.  Each .mat file will contain a collection of bounding boxes extracted from that particular image id by running local detectors across all K classes on that image id. When saving the cached bounding boxes to a such a  .mat file, you should store 2 MATLAB variables : "Detections" and "Scores". The ith row in "Detections" should contain [x1 y1 x2 y2 clsID] of the ith detection window, where the 1st four entries define the bounding box and the 5th entry is your representation of the "ID" of the object whose local detector was used in collecting that window. "Scores" is a column vector whose ith entry corresponds to the local detector's score on the ith window.
For instance, you will create CACHED_DATA_TRAINVAL/0045.mat to store the "Detections" and "Scores" collected across all classes on image ID 0045.
Set the detection threshold low enough so that you collect roughly 700-800 detections across 20 PASCAL classes. the more falso +ves you cache, the more suppressive constraints the model learns.
2. Create 2 folders inside multiobject_context: "LOSS_TRAINVAL" and "FEAT_TRUE_TRAINVAL". The 1st folder will store the 0/1 loss associated with the different detection windows and the latter will store the ground truth feature vector associated with each of the images.  The code as packaged, is configured to compute the  0/1 loss and the ground truth feature vector on the VOC set. 
The naming convention for files in these folders is similar to caching bounding boxes under CACHED_DATA_TRAINVAL. LOSS_TRAINVAL/0045.mat will contain a D x 2 MATLAB variable  named "loss". D is the number of detection windows cached from image id 0045. D(:,1) is the loss for turning "on" a particular window, and D(:,2) is the loss for turning it "off".  FEAT_TRUE_TRAINVAL/0045.mat will contain the ground truth feature vector [Psi(); Phi()]  (described in the paper) based on "fixing" the true +ves for image 0045.mat.


To train:
-----------

1. Run script configure.m: The package is currently configured to learn a model on the PASCAL VOC data. Change the location of vocdevkit_root in line 10 to point to where the VOC Devkit resides on your machine. To train on your own data set other than VOC, comment out code in configure.m  related to VOC (lines 10 -15).  You'll need to substitute the line of code 
ids=textread(sprintf(VOCopts.imgsetpath,'trainval'),'%s');
with your own code that reads the ids of images in your training set into the variable "ids" . Make this change in extract_feat_TP.m and train.m
Additionally, to train on your own data-set, you'll need to define an interface for reading the ground truth bounding boxes in memory equivalent to the VOC interface 
rec=PASreadrecord(sprintf(VOCopts.annopath,ids{id}));. 
This is required in order to decide which of the local detection windows should be "fixed" as true +ves.
Check the VOC PASCAL devkit documentation for details on the VOC PASreadrecord function. Substitute the VOC code with your interface in extract_feat_TP.m

2. Run script main.m: The code implements an "online" version of the cutting plane algorithm described in the paper.

To test:
--------

The package has a script "run_on_test.m" that I used for testing on PASCAL. Use it as a skeleton to augment to your test code to run on your dataset, so that your test function uses the find_MVC_test function. The find_MVC_test function uses cached global variables "Detections" and "Scores" for each test image. These have the same interpretation as in the Pre-Prcessing stage, except that they apply to test images only. The output of find_MVC_test are parameters [E_mex H_wo] . E_mex contains the augmented scores for the detection windows as a result of applying the contextual model. Use these score for computing Prec-Recall curves under the contextual model. The 2nd parameter is just an indicator variable that determines which of the detection windows is "instanced" by the greedy procedure at test time.  


