#
# Easy!  mini tutorial
#
# Make sure the environment is set up correctly
#
# matz 6/18/2013

import os, sys
from datetime import datetime, date
print("Writing environment report file 'python_env.txt'...")
repfile = open('python_env.txt', 'w')
repfile.write('Your environment as found by demo/prerequisites.py.\n')
repfile.write('Generated on ' + str(date.today()) + ' at ' + str(datetime.now()) + '.\n\n')
repfile.write('Python version: ' + str(sys.version_info[0]) + '.' \
               + str(sys.version_info[1]) + '.' \
               + str(sys.version_info[2]) + '\n')
repfile.write('Current working directory: '+ str(os.getcwd()) + '\n' )
repfile.write('Python executable location and version: ' + sys.executable + ', ' + sys.version + '.\n')
repfile.write('Python final sys.path: ' + str(sys.path) + '\n')
repfile.write('Environment variables:\n')
for key in sorted( os.environ ):
    val = os.environ.get( key )
    repfile.write('  ' + key + ' = ' + val + '\n')

def check_degenerate( module, normal ):
    modlen = len(dir(module))
    if modlen<normal:
        print("  ... but the module definition is incomplete!")
        print("  Only {0} functions are known, yet it should be over {1}."
              .format( modlen, normal-1 ) )
        print("  Probably the directory '{0}' is found, but not the actual {1}.py file."
              .format( module.__name__, module.__name__ ) )

print("Trying to import Ice... ")
try:
    import Ice
    print("  succeeded.")
    check_degenerate( Ice, 287 )
except ImportError as ex:
    print("  failed:")
    print("  Please add /your/CVAC_dir/3rdparty/ICE/python to PYTHONPATH.")
    print("  Detailed error message: {0}".format( ex ));

print("Trying to import cvac... ")
try:
    import cvac
    print("  succeeded.")
    check_degenerate( cvac, 157 )
except ImportError as ex:
    print("  failed:")
    print("  Please add /your/CVAC_dir/lib/python to PYTHONPATH.")
    print("  Detailed error message: {0}".format( ex ));

print("Trying to import paths... ")
try:
    import paths
    print("  succeeded.")
except ImportError as ex:
    print("  failed:")
    print("  Please add the directory in which paths.py is located to your PYTHONPATH.")
    print("  If you only have paths.py.in, then you should either obtain a binary")
    print("  distribution package of CVAC or run the CMake build process.")
    print("  Detailed error message: {0}".format( ex ));

print("Trying to import easy... ")
try:
    import easy
    print("  succeeded.")
    check_degenerate( easy, 73 )
except ImportError as ex:
    print("  failed:")
    print("  Please add the directory in which easy.py is located to your PYTHONPATH.")
    print("  That's either /your/CVAC_dir/lib/python or /your/CVAC_dir/src/easy.")
    print("  Detailed error message: {0}".format( ex ));

