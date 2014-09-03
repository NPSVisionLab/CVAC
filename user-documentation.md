---
layout: default
title: EasyCV User Documentation
---

Note: this page is heavily under construction as of 3 Sept 2014.
Check back in a day for updates.

# EasyCV Control Center

Start up the EasyCV Control Center (see the [installation
notes](download.html) for instructions on how to do that).  ![EasyCV
Control Center](images/ControlCenter.png)


Take note of the service's "data" directory.  That's where media files
need to be located in order to be accessible to the services.  If you
have them in another folder, you need to "upload" them to the
FileServer first.  If you would like to change this folder, you need
to edit the config.services file in the Application's directory.

If you haven't already, start with a [high-level
introduction](http://movesinstitute.org/~kolsch/CVAC/index.html) to
Easy! Computer Vision, a glance at the [Frequently Asked
Questions](http://movesinstitute.org/~kolsch/CVAC/faq.html), and the
overview of a [Tutorial at CVPR
2013](http://movesinstitute.org/~kolsch/CVAC/tutorial.html).

Pressing the "start" button will run a few default CV algorithms
(services) locally.  If you get firewall warnings, select "Allow" so
the services can accept service requests from clients.
![Select Allow](images/AllowConnection.png)

Then take a look at the [demo directory](https://github.com/NPSVisionLab/CVAC/tree/devel/demo) which explains most of the features of the high-level Easy! library in simple "mini-tutorials."  You can also see these mini tutorials and their program output [here](http://movesinstitute.org/~kolsch/CVAC/Easy.pdf).  It shows you how to run a detector on files and directories, how to train a detector, how to connect to remote services, and many more things.  Take a look at the underlying `easy.py` library implementation if you want to see the CVAC metadata structure and observe bare-bones interaction between clients and services.

BagOfWordsUSKOCA is specified in config.client as a service running locally, listening on port 10104.  This must correspond to the port number specified in config.service.  This particular service is a pre-trained detector.  The learned model for US, Korean, and Canadian flags is stored in the file specified with `BagOfWordsUSKOCA.DetectorFilename = bowUSKOCA.zip`.

The "data" directory is specified with the CVAC.DataDir property in both the config.client and config.service files.  For local clients and services, this will always be the same directory.  "CTest" is a subdirectory in the CVAC.DataDir.

Stop the services in your EasyCV Control Center.

## Remote services

Test your client against a pre-installed test service:

# Command line and C/C++

If the Easy! library and Python are not your thing, you can also run one of the C/C++ examples and, for example, test files in a directory "data/CTest" with one of the algorithms:

`bin/startIcebox.sh`

`bin/detectorClient BagOfWordsUSKOCA CTest config.client`

`bin/detectorClient NpsBagOfWordsUSKOCA CTest config.client`

`bin/stopIcebox.sh`

**Nps**BagOfWordsUSKOCA is specified in config.client as a service running on vision.nps.edu, listening on port 10104.  This is a service with the same pre-trained detector, so you should get the same results as above.  This time, however, the processing happens on the remote server.

# Troubleshooting

See the [Troubleshooting](troubleshooting.html) page.