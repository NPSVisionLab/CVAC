---
layout: default
title: Integrating your Algorithms
---
# Adding Algorithms to CVAC

## Technical aspects

Naming your algorithm is important and needs to happen at several
places.  Here's a sketch for an algorithm BBB with an adapter named
AAA and the algorithm code in a library called libCCC.dylib:

* config.service: `AAA.Endpoints...`
* config.client: `AAA.Proxy=BBB`
* config.icebox: `IceBox.Service.AAA=bowCCC:create...`
* In the client code:
        ```
        detector = easy.DetectorPrx.checkedCast(self.communicator().propertyToProxy('AAA.Proxy').ice_twoway())
        ```
* In the server code:
        ```
        adapter = self.communicator().createObjectAdapter("AAA")
        sender = ScreenShotDetectionI(self.communicator())
        adapter.add(sender, self.communicator().stringToIdentity("BBB"))
        ```

## Logistics, making your service available

You can share your computer vision algorithm (or your application)
according to your desires.  Here is a recommended schedule that is
both easy because incremental and it provides for maximum information
sharing with the community.  Naturally, issues such as charging for
your services are up to you.

* Create an entry on the appropriate reference site so others can find your algorithm. Update the entry as you go.
** For unlimited-distribution algorithms, put the entry on the github page.
** For US government, DoD, and government contractor distribution, put the entry on the Forge.mil CVAC page.  There, you will find instructions on how to further limit the distribution, if desired.
* Install a service on your network so that non-firewalled access is possible.  That gives others the opportunity to test your algorithm and to explore its capabilities.  You remain in full control of the algorithm, the build environment, and the execution profile.  This makes it easy to deploy since it only needs to run on your platform.  And you don't have to share your code with anyone.
* Provide a shrink-wrapped "algorithm" package that others can download and install on their own computers.  Once installed, their client application can utilize your algorithm right away, on their hardware, without bogging down your systems.  Just as with any "thick client" application, all you have to distribute is the binary code.
* Publish your source code and let others build and install it.
* If your algorithm utilizes a learned model, you can update and publish the model by itself, without re-distributing the algorithm.  For example, a service for the popular Viola-Jones style "Haar" detector that is part of OpenCV remains the same but it can be run with a model for detecting faces or with a model for detecting pedestrians.

## Contributing back your _EasyCV!_ framework changes

After you have a new feature working and you want to contribute it
back to EasyCV you need to follow these steps:

*   Create an account on [www.github.com](http://www.github.com) and log in.
*   Browse to the [NPSVisionLab/CVAC](https://github.com/NPSVisionLab/CVAC) repository and select it.
*   Select the "Fork" button to create a fork of the CVAC project to your account.
*   On your local system,  clone the fork with a "git clone https://github.com/<YOURUSER>/CVAC.git"
*   To be able to update this clone to changes from CVAC you need to add CVAC as a remote using "git remote add CVACorig git://github.com/NPSVisionLab/CVAC.git"
*   To bring your code up to speed with the NPSVisionLab repository's "devel" branch, call "git fetch CVACorig" followed by "git checkout devel" and then followed by "git rebase CVACorig/devel"
*   Changes have to be made on a branch other than "master."  You can switch to the "devel" branch with "git checkout devel" or you can create your own branch.
*   If you made lots of little commits, possibly some undoing earlier commits, and you would like to make merging easier by avoiding a long history of changes, consider "squashing" your many commits into few ones.  Nick Quaranto's [git ready](http://gitready.com/advanced/2009/02/10/squashing-commits-with-rebase.html) blog provides a good explanation on how to do that.
*   Once you are ready to commit a change you first push the change back to your fork.
*   You then hit the PullRequest button on your forked project page.  This will request that all the changes in your forked repository be put into NPSVisionLab/CVAC.  An administrator will evalulate the changes and let to know if they have been accepted.