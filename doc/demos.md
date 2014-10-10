---
layout: default
title: Mini Tutorials
---

# Run the mini tutorials (demos)

In the EasyCV Control Center, make sure that the services have been
started.  Then click the "Terminal Window" button.  Once the window
has opened, change the current directory to the root installation
folder, such as
<br>`cd /Applications/EasyComputerVision.app/Contents/Resources` or
<br>`cd C:\Program Files\EasyComputerVision` or
<br>`cd C:\EasyCV\CVAC_binary-0.8.99-Win7`

A link to the local code and online code is provided below.  You can
also see these mini tutorials and their program output in a
[manual](http://movesinstitute.org/~kolsch/CVAC/Easy.pdf) to an
earlier version of EasyCV.  We might update this if desired.

Invoke each of the following demos with Python like so:<br>
`python demo/prerequisites.py`


* Prerequisites: import cv, import easy:
[prerequisites.py](../demo/prerequisites.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/prerequisites.py))

* Apply a pre-trained detector:
[detect.py](../demo/detect.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/detect.py))

* Train a custom detection model:
[training.py](../demo/training.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/training.py))

* Test your client against a pre-installed detector service and remotely
train a detector for your objects:
[remote_services.py](../demo/remote_services.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/remote_services.py))

* Understand what a RunSet is and how to construct it:
[runset.py](../demo/runset.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/runset.py))

* Utilize a ready-made Corpus, learn about Labels vs. Purposes,
     understand result labels:
[full\_image\_corpus.py](../demo/full_image_corpus.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/full_image_corpus.py))

* Bootstrap a multi-class detector, that is, train it repeatedly on
  on more and more labeled data:
[bootstrapping.py](../demo/bootstrapping.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/bootstrapping.py))

* K-fold cross-validation, detector comparisons:
[evaluation.py](../demo/evaluation.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/evaluation.py))

* Obtain a LabelMe corpus and edit a corpus properties file:
[labelme_corpus.py](../demo/labelme_corpus.py)
(see code [online](https://github.com/NPSVisionLab/CVAC/blob/master/demo/labelme_corpus.py))
