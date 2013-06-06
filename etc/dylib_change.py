'''In OSX dylib files, replace absolute paths to dependencies with
relative paths'''

import sys
import os
import subprocess
from subprocess import Popen, PIPE

def replacePrefix( filepath, currlib, badprefix, goodprefix ):
    '''In file filepath, replace the badprefix in the reference
    currlib path with the goodprefix.'''
    newlib = currlib.replace( badprefix, goodprefix )
    output = subprocess.Popen(['install_name_tool', '-change',
                               currlib, newlib, filepath], stdout=PIPE).communicate()[0]
    return True

def examineFile( fpath, do_replace=False ):
    '''Run otool -L on the fpath and find dylib references to
    absolute paths.  Act if the prefixes of these abs paths are
    known to cause trouble.'''
    knownWrong = [('/opt/Ice-3.4/lib/', '3rdparty/ICE/lib/')
                  ,('/somethingelse', 'replacemewith')
                  ]

    print('examining '+fpath);
    output = subprocess.Popen(['otool', '-L', fpath], stdout=PIPE).communicate()[0]
    # otool first prints the filename, so start at the second line
    for line in output.splitlines()[1:]:
        libname = line.strip().partition('.dylib ')[0]+'.dylib'
        # for dylib fpaths, the dylib references often (always?) start
        # with a link to themselves.  Skip those.
        if libname != fpath and libname.startswith('/'):
            # replace a bunch of known problems
            replaced = False
            for tupl in knownWrong:
                if libname.startswith( tupl[0] ):
                    if do_replace:
                        replaced = replacePrefix( fpath, libname, tupl[0], tupl[1] )
                        print 'replcd: ' + libname
                    else:
                        print '!found: ' + libname
            if not replaced:
                print 'danger? ' + libname
        
def walkDirectory( rootDir, do_replace=False ):
    '''Do just that: walk the specified directory, and for
    all files ending in .dylib, call examineFile.'''
    print('walking from root directory '+rootDir)
    for root, dirs, files in os.walk(rootDir):
        for fname in files:
            if fname.endswith('.dylib'):
                examineFile( os.path.join(root, fname), do_replace )

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
    args = parser.parse_args()
    filenames = args.filenames
    do_replace = args.do_replace

except ImportError:
    # argparse is new in Python 2.7
    filenames = ['.']
    do_replace = False
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
        walkDirectory( fname, do_replace )
    else:
        # examine individual file
        examineFile( fname, do_replace )
