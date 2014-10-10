---
layout: default
title: Frequently Asked Questions
---

# Frequently Asked Questions and Answers

## How is EasyCV different from OpenCV? Isn't OpenCV the de-facto standard for vision libraries?

* OpenCV is a library for vision developers
* EasyCV is for vision users and focused on metadata instead on implementation of algorithms
* EasyCV provides an increasing amount of metadata for automation,
* its network service interface gives access to free as well as non-free and non-open algorithms and data sets,
* and it avoids the burden on the algorithm supplier to comply to a given language or architecture, thereby permitting a more modular inclusion into the pool of algorithms.
* Under the hood of EasyCV is a unified metadata exchange specification that enables seamless communication between algorithms implemented in OpenCV or any other vision library

## What is CVAC and how does it relate to EasyCV?

CVAC is a data definition and service description middleware (framework) that abstracts from CV data and tasks. EasyCV is a higher-level abstraction that hides most of the generality and hence complexity of CVAC. CVAC got its name from the Computer Vision Algorithm Collection, a DARPA-sponsored effort to collect, make available, and reuse algorithms that the DOD has sponsored and paid for, thereby fostering collaboration, avoiding re-inventing the wheel and spending tax dollars efficiently. The CVAC framework derived from the desire to actually use the algorithms stored in the CVAC collection, as easily as possible. EasyCV is tied to the CVAC framework, in that it allows algorithms to be reused, well, easily. EasyCV and the CVAC framework enable reuse of any algorithm, DOD sponsored or not. They also enable easy access to a wealth of labeled images and videos.

## Who should attend the CVPR Tutorial?

* Students and researchers in CV wanting to utilize annotated datasets and to compare their algorithms with others, and/or just want to leave behind a reusable software package with a well-defined interface,
* Professionals interested in learning about ways to explore the capabilities of CV in an approachable, technically comprehensible way,
* Program managers and contract officers who want to formalize and proceduralize requirements specifications for CV functionality,
* Users and developers of computer vision algorithms who want to leverage other work, get more exposure for their work, or want to conveniently access a growing collection of algorithms;
* Anybody interested in applying the “scientific method” principle of evaluation and comparability in a convenient fashion to computer vision, and anyone who shares the vision of a marketplace for computer vision methods.

## What will this 4-hour tutorial teach me?

After attending this tutorial, you will have gained an appreciation for the features and benefits of EasyCV Computer Vision. You will be able to

* Program EasyCV and solve computer vision problems such as object detection, object recognition, shot boundary detection, and video analysis
* Create a Corpus of labeled data from scratch and by accessing already annotated data sets such as Caltech101, TRECVID, and LabelMe
* Understand how to adapt an existing algorithm for EasyCV using the CVAC utility library
* Build an object detector for a variety of objects in image and video data
* Evaluate algorithms for your data and your application
* Understand the differences between OpenCV, other libraries, and the objectives of EasyCV
* Invoke vision methods for standard vision problems with a few lines of code
* Wrap your own vision algorithm and make it "easy" in one day of coding
* Compare and evaluate algorithms through the same interface, with the same data
* Clearly distinguish syntactic issues from the "semantic gap" for automated image understanding.

## Why should I use EasyCV?

* Open-source framework code
* Easy to collaborate
* Everyone can contribute algorithms that integrate seamlessly
* Stable interface to data and algorithms; API changes are subject to careful design choices
* If desired, algorithms + trained models + data can be kept proprietary information
* Catch the wave of recent surge in interest in computer vision without needing to learn or understand intricate difficulties in detail.
* Advertise your algorithm by making it easily accessible and available as a service.

## What kind of computer vision problems is EasyCV designed for?

EasyCV Computer Vision has been designed to cover a broad range of image understanding tasks. Image processing is not one of the objectives. Currently, there is tested support for:

* object detection such as face detection or pedestrian detection or other binary visual classification problems
* object recognition such as biometric face recognition or other multi-class identification problems
* scene classification, e.g. outdoor, indoor, restaurant, portrait
* action recognition, e.g. walking, sitting, eating, typing, playing an instrument
* object tracking, such as soccer players or cars
* image segmentation, that is, division of an image into regions; this produces image labels and hence is not merely image processing
* shot boundary detection, that is, temporal video segmentation

We intend to extend the library to cover 3D aspects such as

* 3D reconstruction, or shape-from-X, such as depth from stereo, as it produces depth “labels”
* image mosaicing (recovers a camera model parameterization)
* pose (6DOF) and posture (articulated body configuration) recognition

## What questions have you not been asked yet?

* Who uses Easy and CVAC?
* What does EasyCV currently connect?
* What problems does EasyCV solve?
* What does EasyCV enable me to do?
