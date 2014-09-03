---
layout: default
title: Downloading EasyCV
---


# CVAC Binaries

You can obtain binary installers for several major operating systems which include all required components to run CVAC, including [Python 2](http://www.python.org/download/releases/) and [NumPy](http://www.numpy.org/).  This will allow you to perform all tasks including local detection, local training, and connection to remote services.  (Some services are available in Python and in Java.  If you would like to use the Java versions, please install any Java JRE.)

Binary packages are available for:
* [Windows 7, XP](http://www.movesinstitute.org/~kolsch/CVAC/Download.php): see Win7 files
* [Windows 8](http://www.movesinstitute.org/~kolsch/CVAC/Download.php): see Win8 files
* [Mac OSX](http://www.movesinstitute.org/~kolsch/CVAC/Download.php): see Darwin files. [Mac OSX Installation Instructions](http://movesinstitute.org/~kolsch/CVAC/macosx_install_instructions.html)

# Cloning the source code

If you want to add your own services or modify other aspects of the code, you need to obtain the source code.  The CVAC code is version-controlled with Git and lives on GitHub from where you can download and build your copy.  There are several ways to download CVAC, or to "clone the repository" in Git terminology.  The easiest is to use the web interface to clone:

CVAC on GitHub:  [https://github.com/NPSVisionLab/CVAC](https://github.com/NPSVisionLab/CVAC)
Click the appropriate "Clone" button at the top.

The source code can be obtained by cloning the repository, either via
the "Clone" button on the [github
repository](https://github.com/NPSVisionLab/CVAC/wiki) or via the
command line git tool:

`git clone https://github.com/NPSVisionLab/CVAC.git localCvacDirectory`


# Alternative ways to obtain the CVAC source

1) Via the git command line tools.  Simply call the following command, which will create a directory MyCVAC and create a local clone of the CVAC code repository in that directory.

  `git clone https://github.com/NPSVisionLab/CVAC.git MyCVAC`

2) Via another Git client such as 'Git Extensions' for Windows, a package that can be installed from http://gitextensions.googlecode.com/files/GitExtensions244SetupComplete.msi

In Git, you must add the online CVAC project as a remote repository:  Open menu item 'Remotes' --> 'Manage remote repositories'.  Press the 'New' button to create a new entry.  Set the Name and URL fields.  Name should be something like 'CVAC_original', to differentiate from the local copy you will create and edit. The URL field should reference the NPSVisionLab user's version of CVAC: https://github.com/NPSVisionLab/CVAC.git  To get a local copy of CVAC to use on your machine, create a local fork/clone.  Open menu item 'Github' --> 'Fork/Clone repository'.

