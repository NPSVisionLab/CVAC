---
layout: default
title: Easy Computer Vision
---

Easy Computer Vision is a library of tools to simplify the high-level
operations on computer vision methods.  It provides access to
algorithms in the CV Algorithm Collection (CVAC) through well-defined
interfaces, it links annotation tools (LabelMe, VATIC) to algorithms,
and it permits creation of new detectors and their performance
evaluation.  You can connect to remotely running vision services, run
algorithms as services on your computer, or embed algorithms into your
program space.  You can obtain and share a "corpus" of labeled objects
for training or testing.  And you can easily exchange one algorithm
for another without modifying your code (much).

## Easy! Computer Vision ...

... is a framework for anybody who wants to *tap into the power for
computer vision quickly and easily:* application developers that would
like to use computer vision, computer vision algorithm developers,
contracting officers, and program managers.

... *connects algorithms with data sets,* e.g. from the Computer Vision
Algorithm Collection (CVAC) or from OpenCV and from the Caltech101
corpus or LabelMe.

... enables transparent remote service invocation, cross-language
communication, and a *plug-and-play* approach to model training,
testing, and deployment.

# Download and Install

Please see the [download and installation page](download.html) for
binary installers for various operating systems, for installation
instructions, as well as for source code download instructions.

# Documentation

Start with the [user documentation](user-documentation.html) which
includes an [EasyCV Quickstart](user-documentation.html#quickstart)
guide on how to use EasyCV and run your first examples.  Of
course, there is a [FAQ](faq.html).

In addition, there are several tutorials and the API documentation:

* [Mini tutorials](demos.html) (demos) to get you started quickly;
* The Python [EasyCV](html/namespaceeasy.html) library API for high-level functionality;
* The [cvac:: namespace](html/namespacecvac.html) for data and service definitions;
* Documentation for [building from source](building.html) including
  [3rd-party dependencies](dependencies.html);
* Instructions for [adding your algorithms](integrating.html)
  to EasyCV and CVAC (see also Contributing below);
* [Frequently Asked Questions](http://movesinstitute.org/~kolsch/CVAC/faq.html);
* Overview of a [Tutorial at CVPR 2013](http://movesinstitute.org/~kolsch/CVAC/tutorial.html),
  including a better-formatted [manual](http://movesinstitute.org/~kolsch/CVAC/Easy.pdf)
  to an earlier version of EasyCV.
* And, last but not least, some [Troubleshooting](troubleshooting.html) help.

# Contributing

More detailed instructions are [here](integrating.html).  In summary:

1. Fork it. (Press the ["Fork" button](https://github.com/NPSVisionLab/CVAC))
2. Create a branch off the devel branch (`git checkout -b my_CVAC devel`)
3. Commit your changes (`git commit -am "Added parsing for MyAnnotations"`)
4. Push to the branch (`git push origin my_CVAC`)
5. Open a [Pull Request](https://github.com/NPSVisionLab/CVAC/pulls)
6. Wait for the request to be merged
