#!/usr/bin/python2.6
import sys
import inspect
import os
import stat
import tempfile

# get this file's path:
scriptfname = inspect.getfile(inspect.currentframe())
scriptpath  = os.path.dirname(os.path.abspath( scriptfname ))
installpath = os.path.abspath(scriptpath+'/../Resources')

sys.path.append(installpath+'/python/easy')
sys.path.append(installpath+'/demo')
sys.path.append(installpath+'/3rdparty/ICE/python')
sys.path.append(installpath+'/python')

# fill in the INSTALLDIR:
# export INSTALLDIR=....
def patchInstallDir( filename, installpath ):
    with open(filename) as inf:
        outfile = tempfile.mkstemp(text=True)
        #    outfile = mkstemp()
        with os.fdopen(outfile[0],"w") as outf:
            for line in inf:
                if line.strip()=='export INSTALLDIR=':
                    line = 'export INSTALLDIR='+installpath+'\n'
                outf.writelines(line)
            os.rename( outfile[1], filename )
            os.chmod( filename, stat.S_IRUSR|stat.S_IWUSR|stat.S_IXUSR|stat.S_IXGRP|stat.S_IXOTH )

patchInstallDir( installpath+'/bin/startServices.sh', installpath )
patchInstallDir( installpath+'/bin/stopServices.sh', installpath )

# open the simple GUI that displays system information
# and permits startup of the services
import gui
