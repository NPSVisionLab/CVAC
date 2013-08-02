Virtual Machine Instructions
===================================

What's on the DVD?
-------------------------------------------
The tutorial DVD contains a Ubuntu virtual machine with
*Easy!* installed and configured.  You can run *Easy!* by installing the
Oracle VirtualBox on your "host" operating system (OS) and then just
opening the Ubuntu "guest" OS within that program.
That way, you can get started quickly and you only have to install a
single program (VirtualBox) on your computer.

The DVD contains:

* VirtualBox installers for Windows, Apple OSX, and Ubuntu.

* A completely installed and configured Ubuntu OS image, containing the *Easy!* framework.



Installation
------------------
Follow these steps to first install the VirtualBox for your respective
*host* operating system, then start up the Ubuntu *guest* inside
VirtualBox. 

1. Insert the DVD and find the appropriate VirtualBox installer for
your *host* operating system.

2. Run the installer and follow the instructions to install
VirtualBox on your computer.  Take note of the directory which you
select to store virtual machines in, usually a subdirectory in your
home folder called "VirtualBox VMs."

3. Copy the EasyComputerVision folder from the DVD into this
"VirtualBox VMs" folder.

4. Edit the permissions on all files in the EasyComputerVision folder
**and the folder itself**
to permit "user write" operations.  The instructions differ by
operating system - please let us know if you have questions.

5. Start VirtualBox.  You can do this from Applications or the Start
menu or by double-clicking on the EasyComputerVision.vbox file in your
"VirtualBox VMs" folder.  Take care not to double-click on items on
the DVD.

6. VirtualBox is starting up the Ubuntu *guest* and you should see the
Ubuntu Desktop soon.  If you need to log on, the user name and
password are both "easy" without quotes and all in lower-case.

7. Click on the "Terminal" icon, the black rectangle with the ">"
sign.  Let's call the terminal window that pops up the *service
terminal.*  At its command prompt, type this, followed by return:

  ``> cd CVAC``
  
  ``> bin/startIcebox.sh``

.. raw:: latex

   \newpage

8. Right-click on the Terminal icon again, click on New Terminal.  In
this *client terminal,* type this, followed again by return:

  ``> cd CVAC``
  
  ``> python demo/prerequisites.py``

If you none of the commands have produced any error messages, you are
ready to follow along with the interactive part of the tutorial.

Advanced VirtualBox Tips
------------------------
The following is NOT required for this tutorial, but might make future
tests easier.

You can connect from the Ubuntu *guest* to the internet, but you
cannot connect from *guest* to *host* or from *host* to *guest.* If
you would like to open an **ssh** connection from your *host* to the
*guest,* you need to enable Port Forwarding.  Again, this is not
required for the tutorial.

  * In VirtualBox for the Easy Computer Vision virtual machine, select
    Devices --> Networks --> Adapter 1 --> Advanced --> Port
    Forwarding.

  * Add a rule as follows: name=easy-ssh, host
    port=2222, guest port=22.  Leave both host IP and guest IP blank.
    Save these settings.
    
  * From your *guest* OS, you can now ssh to the *host* by calling
    "ssh -P 2222 easy\@localhost".  Note that the -P might be
    lower-case, depending on your OS.
