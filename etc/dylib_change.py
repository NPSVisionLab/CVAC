'''In OSX dylib files, replace absolute paths to dependencies with
relative paths.  If you want to read up about this, look at the
dyld man page:
https://developer.apple.com/library/mac/#documentation/Darwin/Reference/ManPages/man1/dyld.1.html
'''

import sys
import os
import subprocess
from subprocess import Popen, PIPE

# global variables
filenames = ['.']
do_replace = False
idname = False

def replacePrefix( filepath, currlib, badprefix, goodprefix ):
    '''In file filepath, replace the badprefix in the reference
    currlib path with the goodprefix.'''
    newlib = currlib.replace( badprefix, goodprefix )
    if idname:
        # modify the filepath's "install name"
        modwhat = '-id'
        output = subprocess.Popen(['install_name_tool', modwhat, 
                                   newlib, filepath], stdout=PIPE).communicate()[0]
    else:
        # modify the filepath's references to dependencies
        modwhat = '-change'
        output = subprocess.Popen(['install_name_tool', modwhat, 
                                   currlib, newlib, filepath], stdout=PIPE).communicate()[0]
    return True

def examineFile( fpath ):
    '''Run otool -L on the fpath and find dylib references to
    absolute paths.  Act if the prefixes of these abs paths are
    known to cause trouble.'''

    # this should be a command line option but it's not:
    # for libraries (dylib), the substitution should be (old, new):
    #   ('/opt/Ice-3.4/lib/', '@loader_path/')
    # for executables in Ice/bin, the substitution should be (old, new):
    #   ('/opt/Ice-3.4/lib/', '@loader_path/../lib/')
    # for shared libraries in Ice/python, the substitution should be (old, new):
    #   ('/opt/Ice-3.4/lib/', '@loader_path/../lib/')
    knownWrong = [('/opt/Ice-3.4/lib/', '@rpath/3rdparty/ICE/lib/')
                  ,('/another/path/to/lib/', '@loader_path/../lib/')
                  ]

    print('examining '+fpath);
    if idname:
        output = subprocess.Popen(['otool', '-D', fpath], stdout=PIPE).communicate()[0]
    else:
        output = subprocess.Popen(['otool', '-L', fpath], stdout=PIPE).communicate()[0]
    # otool first prints the filename, so start at the second line
    for line in output.splitlines()[1:]:
        libname = line.strip().partition('.dylib')[0]+'.dylib'
        # replace a bunch of known problems
        found = False
        for tupl in knownWrong:
            if libname.startswith( tupl[0] ):
                if do_replace:
                    found = replacePrefix( fpath, libname, tupl[0], tupl[1] )
                    print 'replcd: ' + libname
                else:
                    found = True
                    print '!found: ' + libname
        if libname.startswith('/') and not found:
            print 'danger? ' + libname
        
def walkDirectory( rootDir ):
    '''Do just that: walk the specified directory, and for
    all files ending in .dylib, call examineFile.'''
    print('walking from root directory '+rootDir)
    for root, dirs, files in os.walk(rootDir):
        for fname in files:
            if fname.endswith('.dylib'):
                examineFile( os.path.join(root, fname) )

#
# start of main function;   parse command line arguments
#
try:
    import argparse
    parser = argparse.ArgumentParser(description='List or replace absolute paths to dylibs.')
    parser.add_argument('filenames', metavar='filename', type=str, default=['.'], nargs='*',
                        help='individual file or directory')
    parser.add_argument('--replace', dest='do_replace', action='store_true', default=False,
                        help='run install_name_tool to replace paths; otherwise just list')
    parser.add_argument('--id', dest='idname', action='store_true', default=False,
                        help='list or replace library "install names" instead of its dependencies')
    args = parser.parse_args()
    filenames = args.filenames
    do_replace = args.do_replace
    idname = args.idname

except ImportError:
    # argparse is new in Python 2.7
    print('You should run this with python2.7 for complete functionality.')
    if len(sys.argv)>1:
        if sys.argv[1]=='--replace':
            do_replace = True
        else:
            filenames = [sys.argv[1]]
    if len(sys.argv)>2:
        filenames = sys.argv[2:]

#
# if main argument is a file, just examine that,
# otherwise walk the directory that's specified, default is '.'
#
for fname in filenames:
    if os.path.isdir( fname ):
        walkDirectory( fname )
    else:
        # examine individual file
        examineFile( fname )
