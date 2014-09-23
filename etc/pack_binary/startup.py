import sys
import inspect
import os
import stat
import tempfile
import getopt
import subprocess
import installNumpy

# fill in the INSTALLDIR and PYTHONEXE
# export INSTALLDIR=....
def patchInstallDir( filename, installpath, pythonexec=None ):
    with open(filename) as inf:
        outfile = tempfile.mkstemp(text=True)
        #    outfile = mkstemp()
        with os.fdopen(outfile[0],"w") as outf:
            for line in inf:
                if line.strip()=='export INSTALLDIR=':
                    line = 'export INSTALLDIR='+installpath+'\n'
                if pythonexec != None:
                    if line.find('export PYTHONEXE=') >= 0:
                        line = 'export PYTHONEXE='+pythonexec+'\n'
                outf.writelines(line)
            os.rename( outfile[1], filename )
            os.chmod( filename, stat.S_IRUSR|stat.S_IWUSR|stat.S_IXUSR|stat.S_IXGRP|stat.S_IXOTH )

#startup.py [-s (silent, no dialogs)] [-v <virtual_dir>] [-p <python exe>]
if __name__ == '__main__':

    argv = sys.argv[1:]
    silent = False
    virtualDir = None
    pythonExec = None

    try:
        opts, args = getopt.getopt(argv, "sv:p:")
    except getopt.GetoptError:
        print("Invalid command line option")
        print(sys.argv[0] + ' [-s] [-v <virtualenv dir>] [-p <python.exe>]')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-s':
            silent = True
        elif opt == '-v':
            virtualDir = arg
        elif opt == '-p':
            pythonExec = arg

    # If the pythonExec was not passed in then set the python
    # executing this script as the python to use.
    if pythonExec == None:
        pythonExec = sys.executable



    # get this file's path:
    scriptfname = inspect.getfile(inspect.currentframe())
    scriptpath  = os.path.dirname(os.path.abspath( scriptfname ))
    installpath = os.path.abspath(scriptpath+'/../Resources')

    if installpath+'/python/easy' not in sys.path:
	    sys.path.append(installpath+'/python/easy')
    if installpath+'/demo' not in sys.path:
	    sys.path.append(installpath+'/demo')
    if installpath+'/3rdparty/ICE/python' not in sys.path:
	    sys.path.append(installpath+'/3rdparty/ICE/python')
    if installpath+'/python' not in sys.path:
	    sys.path.append(installpath+'/python')
    # We need python path to point to our virtual site
    if installpath+'/virt/lib/python2.7/site-packages' not in sys.path:
	    sys.path.append(installpath+'/virt/lib/python2.7/site-packages')

    #checking python version should now be handled by setup.py
    # and install_requires.
    #check if we have the correct python version
    versionNum = sys.hexversion
    #if versionNum < 0x02070000 or versionNum >= 0x02080000:
        # Don't have the correct python version so lets run the 
        # virtual env

    virtualDir = installpath + '/virt'
    activate_this = virtualDir + '/bin/activate_this.py'
    installVE = True
    pathenv = os.getenv("PATH")
    tempPath = virtualDir + '/bin'
    if tempPath not in pathenv:
        os.putenv("PATH", tempPath + ':' + pathenv)
        pathenv = os.getenv("PATH")
    if os.path.isfile(activate_this):
        # We might have a virtual env lets try to activate it.
        try:
            execfile(activate_this, dict(__file__=activate_this))
            # if the activation worked then we have a sys real_prefix
            if hasattr(sys, 'real_prefix') == True:
                print("Using existing virtual env in " + virtualDir)
                installVE = False
            else:
                print("Installing virtual env")
        except:
            print("Installing virtual env")
            pass
    # Always create a virtual env so we don't need root access to install
    # easy and ice packages
    if installVE == True:
        print(pythonExec + " " +  installpath+'/3rdparty/virtualenv-1.11.6/virtualenv.py ' + virtualDir)
        subprocess.call([pythonExec, 
             installpath+'/3rdparty/virtualenv-1.11.6/virtualenv.py', 
             '--no-site-packages',
             '--never-download',
             virtualDir])

        pythonExec = virtualDir+'/bin/python'
        #next we install the ice and easy packages
        os.chdir(installpath + '/python/easyPkg')
        print("installing easyPkg")
        subprocess.call([pythonExec, 'setup.py', 'install'])
        subprocess.call([pythonExec, 'setup.py', 'clean --all'])
        os.chdir(installpath + '/python/icePkg')
        print("installing icePkg")
        subprocess.call([pythonExec, 'setup.py', 'install'])
        subprocess.call([pythonExec, 'setup.py', 'clean --all'])
        if os.path.isdir(installpath + '/3rdparty/libsvm'):
            os.chdir(installpath + '/3rdparty/libsvm')
            print("installing libsvm")
            subprocess.call([pythonExec, 'setup.py', 'install'])

        patchInstallDir( installpath+'/bin/startServices.sh', installpath, 
                         pythonexec=pythonExec)
        patchInstallDir( installpath+'/bin/stopServices.sh', installpath,
                         pythonexec=pythonExec)
        #activate the virtual env
        execfile(activate_this, dict(__file__=activate_this))


    # Setup our env
    envstr = ""
    for pstr in sys.path:
        if pstr != None and pstr != "":
            envstr = envstr + ':'+ pstr
    #print("envstr " + envstr)
    if "EasyCV-" not in envstr:
	    envstr = installpath+"/virt/lib/python2.7/site-packages/EasyCV-0.8.0-py2.7.egg:" + envstr
    if "EasyCVIce-" not in envstr:
	    envstr = installpath+"/virt/lib/python2.7/site-packages/EasyCVIce-0.8.0-py2.7.egg:" +envstr
    dystr = os.getenv('DYLD_LIBRARY_PATH')
    if dystr == None:
        dystr = ""
    dystr = installpath + '/3rdparty/ICE/lib:' + dystr
    env = {'PYTHONPATH':envstr, 'PATH': pathenv, 'DYLD_LIBRARY_PATH':dystr}

    if installVE == True:
        # See if Numpy is installed in our virtual env and install if not
        # We need execute with pythonExec and not current python
        subprocess.call([installpath + '/virt/bin/python', 
                     installpath+ '/../MacOS/installNumpy.py'], 
                     env=env)
        # For other users to use EasyCV they need permission to write the
        # lock file so check if this is a /Application install and
        # if it is then try and change the permissions on the install dir.
        if installpath.startswith("/Applications") == True:
            os.chmod(installpath, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    # open the simple GUI that displays system information
    # and permits startup of the services using the correct python
    print("silent {0}".format(silent))
    if silent == False:
        subprocess.Popen([pythonExec, installpath+ '/src/easy/gui.py'], 
                              env=env)
