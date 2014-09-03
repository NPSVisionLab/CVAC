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

# Download and Install

Please see the [download page](download.html) for binary installers
for various operating systems, for installation instructions, as well
as for source code download instructions.

# Documentation

Take a look at the [user documentation](user-documentation.html) on
how to start EasyCV and how to run your first examples.

Several tutorials and API documentation are also available:

* [Mini tutorials](demos.html) to get you started quickly;
* The Python [EasyCV](html/namespaceeasy.html) library API for high-level functionality; and
* The [cvac:: namespace](html/namespacecvac.html) for data and service definitions.

# Contributing

More detailed instructions are on the [Wiki](https://github.com/NPSVisionLab/CVAC/wiki).  In summary:

1. Fork it. (See the "Fork" button above.)
2. Create a branch off the devel branch (`git checkout -b my_CVAC devel`)
3. Commit your changes (`git commit -am "Added parsing for MyAnnotations"`)
4. Push to the branch (`git push origin my_CVAC`)
5. Open a [Pull Request](https://github.com/NPSVisionLab/CVAC/pulls)
6. Wait for the request to be merged
