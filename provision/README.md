# EasyCV Provisioning

A description of how to provision virtual machines for EasyCV. This
will eventually include both build, test, and production environments,
for both client-only installations and those combined with servers.

## Overview

You need Vagrant and a VM provider, currently tested only with
VirtualBox.  You also need the ChefDK and the Vagrant-Berkshelf plugin
from [https://github.com/berkshelf/vagrant-berkshelf].  You might also
need Ruby, Berkshelf, and Omnibus, although that will likely be
installed automatically.  Note that bringing up the VM for the first
time will download a large VM image from the web, and that
provisioning the VM will also download further packages (apt-get).

The Chef cookbook is currently embedded in the source code
repository. That might not be the best way to do things in the long run.

## Instructions

Get the EasyCV branch that currently contains the code to provision. You
will be working on this working copy both from your host and the guest VM
if you are debugging the VM provisioning.

`git clone https://github.com/NPSVisionLab/CVAC.git /host/user/EasyCV`
`git checkout feature-clientbuild`
`cd provision/cookbooks/easycv-client-ipynb`

Start the VM and provision it:

`vagrant up client`

Take note in case there is a port collision. Remember the last number:

`==> client: Fixed port collision for 8888 => 8989. Now on port 2201.`

If all goes well, you should be able to open a browser on your host and
see the iPython Notebook at:

http://127.0.0.1:8989 (or the collision-fixed port)

You can also ssh into the machine with

`vagrant ssh client`


## Debugging

Create a ~/Vagrantfile with the following contents (adjust for your host):

`config.vm.synced_folder "/host/user/EasyCV", "/home/vagrant/EasyCV"`

Then reload your VM and ssh into it:

`vagrant reload client`
`vagrant ssh client`

You can now build on the guest VM but use the sync'ed file system on
your host. So you can edit files in your IDE on the host, run git
commands on the host, but use the compiler and environment on the
guest. Note that the existance of the 3rdparty subfolder will cause
issues on Ubuntu - you might want to work in a different working copy
than an installation on OSX or Windows (which have the 3rdparty
folder).


## Q & A

### What should be a cookbook versus a recipe?

### What is the proper directory structure for Vagrant and Chef?
    Nested or separate hierarchies?

