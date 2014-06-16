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

    print('install path ' + installpath)
    sys.path.append(installpath+'/python/easy')
    sys.path.append(installpath+'/demo')
    sys.path.append(installpath+'/3rdparty/ICE/python')
    sys.path.append(installpath+'/python')

    #check if we have the correct python version
    versionNum = sys.hexversion
    if versionNum < 0x02060800 or versionNum >= 0x02070000:
        # bring up the gui to install python
        import installPython

    if virtualDir != None:
        # Create a virtual env at virtualDir and change pythonExec to point
        # to that
        subprocess.Popen([pythonExec,' -m virtualenv ' + virtualDir])
        pathenv = os.getenv("PATH")
        os.putenv("PATH", virtualDir+'/bin:$PATH')
        pythonExec = virtualDir+'/bin/python'

        

    patchInstallDir( installpath+'/bin/startServices.sh', installpath, 
                     pythonexec=pythonExec)
    patchInstallDir( installpath+'/bin/stopServices.sh', installpath,
                     pythonexec=pythonExec)

    try:
        import numpy
    except ImportError as ex:
        #bring up the gui to install numpy
        installNumpy.doInstall(silent=silent)

    # open the simple GUI that displays system information
    # and permits startup of the services using the correct python
    print("silent {0}".format(silent))
    if silent == False:
        envstr = ""
        for pstr in sys.path:
            if pstr != None and pstr != "":
                envstr = envstr + ':'+ pstr
        env = {'PYTHONPATH':envstr}
        subprocess.Popen([pythonExec, installpath+ '/src/easy/gui.py'], 
                              env=env)
