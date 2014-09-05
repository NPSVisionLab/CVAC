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
<br>`cd C:\Program Files\EasyComputerVision`

Now you can run each of the following demos by invoking it with
Python:<br>
`python prerequisites.py`

A link to the local code and online code is provided below.  You can
also see these mini tutorials and their program output in a
[manual](http://movesinstitute.org/~kolsch/CVAC/Easy.pdf) to an
earlier version of EasyCV.  We might update this if desired.


* Prerequisites: import cv, import easy:
[prerequisites.py](../demo/prerequisites.py)
[(online)](https://github.com/NPSVisionLab/CVAC/blob/master/demo/prerequisites.py)

* Apply a pre-trained detector:
[detect.py](../demo/detect.py)
[(online)](https://github.com/NPSVisionLab/CVAC/blob/master/demo/detect.py)

* Test your client against a pre-installed test service:
[detect_remote.py](../demo/detect_remote.py)
[(online)](https://github.com/NPSVisionLab/CVAC/blob/master/demo/detect_remote.py)

* Create a RunSet for detection and evaluation:
[detect_runset.py](../demo/detect_runset.py)
[(online)](https://github.com/NPSVisionLab/CVAC/blob/master/demo/detect_runset.py)

* Train a custom detection model:
[training.py](../demo/training.py)
[(online)](https://github.com/NPSVisionLab/CVAC/blob/master/demo/training.py)

* Utilize a ready-made Corpus, learn about Labels vs. Purposes,
     understand result labels:
[full_image_corpus.py](../demo/full_image_corpus.py)
[(online)](https://github.com/NPSVisionLab/CVAC/blob/master/demo/full_image_corpus.py)

* Obtain a LabelMe corpus and edit a corpus properties file:
[labelme_corpus.py](../demo/labelme_corpus.py)
[(online)](https://github.com/NPSVisionLab/CVAC/blob/master/demo/labelme_corpus.py)
