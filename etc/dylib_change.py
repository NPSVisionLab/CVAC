'''In OSX dylib files, replace absolute paths to dependencies with
relative paths'''

import sys
import os
import subprocess
from subprocess import Popen, PIPE

def replacePrefix( filepath, currlib, badprefix, goodprefix ):
    newlib = currlib.replace( badprefix, goodprefix )
    output = subprocess.Popen(['install_name_tool', '-change',
                               currlib, newlib, filepath], stdout=PIPE).communicate()[0]
    return True

def examineDylib( fpath ):
    knownWrong = ['/opt/Ice-3.4/lib/']

    print('examining '+fpath);
    output = subprocess.Popen(['otool', '-L', fpath], stdout=PIPE).communicate()[0]
    # otool first prints the filename, then the dylib
    # refers to itself.  So starting at index 2, we're getting dependencies.
    for line in output.splitlines()[2:]:
        libname = line.strip().partition('.dylib ')[0]+'.dylib'
        if libname.startswith('/'):
            # replace a bunch of known problems
            replaced = False
            for prefix in knownWrong:
                if libname.startswith( prefix ):
                    replaced = replacePrefix( fpath, libname, prefix, 'lib/' )
            if not replaced:
                print 'danger? ' + libname
        

rootDir = '.'
if len(sys.argv)>1:
    rootDir = sys.argv[1]
print('walking from root directory '+rootDir)
for root, dirs, files in os.walk(rootDir):
    for fname in files:
        if fname.endswith('.dylib'):
            examineDylib( os.path.join(root, fname) )
