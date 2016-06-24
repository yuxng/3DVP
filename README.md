# 3DVP: Data-Driven 3D Voxel Patterns for Object Category Recognition

Created by Yu Xiang at CVGL at Stanford University.

### Introduction

We propose a novel object representation, 3D Voxel Pattern (3DVP), that jointly encodes the key properties of objects including appearance, 3D shape, viewpoint, occlusion and truncation. We discover 3DVPs in a data-driven way, and train a bank of specialized detectors for a dictionary of 3DVPs. The 3DVP detectors are capable of detecting objects with specific visibility patterns and transferring the meta-data from the 3DVPs to the detected objects, such as 2D segmentation mask, 3D pose as well as occlusion or truncation boundaries. The transferred meta-data allows us to infer the occlusion relationship among objects, which in turn provides improved object recognition results.

### License

3DVP is released under the MIT License (refer to the LICENSE file for details).

### Citing 3DVP

If you find SubCNN useful in your research, please consider citing:

    @incollection{xiang2015data,
        author = {Xiang, Yu and Choi, Wongun and Lin, Yuanqing and Savarese, Silvio},
        title = {Data-Driven 3D Voxel Patterns for Object Category Recognition},
        booktitle = {IEEE Conference on Computer Vision and Pattern Recognition (CVPR)},
        pages = {1903--1911},
        year = {2015}
    }

### File Organization
1. Geometry: scripts to voxelize 3D CAD models. 
The main function is cad_train.m.


