EasyCV - Easy Computer Vision
====

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

Download
------------

Please see the [Wiki pages](https://github.com/NPSVisionLab/CVAC/wiki)
for complete instructions.  The
[Downloads](https://github.com/NPSVisionLab/CVAC/wiki/Downloading-CVAC)
page also lists binary installers for various operating systems.

The source code can be obtained by cloning the repository, either via
the "Clone" button above or via the command line git tool (`git clone
https://github.com/NPSVisionLab/CVAC.git localCvacDirectory`).  Again,
more detail is on the [Wiki
pages](https://github.com/NPSVisionLab/CVAC/wiki).

Documentation
------------

Take a look at the online documentation [overview and introduction]
(https://github.com/NPSVisionLab/CVAC/wiki/User-Documentation) on how
to start EasyCV and how to run your first examples.

Several tutorials and API documentation are available locally after
installation:

* [Mini tutorials](../../demo/demos.html) to get you started quickly
 ([online](https://github.com/NPSVisionLab/CVAC/tree/master/demo));
* The Python [EasyCV](namespaceeasy.html) library API for high-level functionality; and
* The [cvac:: namespace](namespacecvac.html) for data and service definitions.

Contributing
------------
More detailed instructions are on the [Wiki](https://github.com/NPSVisionLab/CVAC/wiki).  In summary:

1. Fork it. (See the "Fork" button above.)
2. Create a branch off the devel branch (`git checkout -b my_CVAC devel`)
3. Commit your changes (`git commit -am "Added parsing for MyAnnotations"`)
4. Push to the branch (`git push origin my_CVAC`)
5. Open a [Pull Request](https://github.com/NPSVisionLab/CVAC/pulls)
6. Wait for the request to be merged
