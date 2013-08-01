#
# Easy!  mini tutorial
#
# Make sure the environment is set up correctly
#
# matz 6/18/2013

print("Trying to import Ice... ")
try:
    import Ice
    print("  succeeded.")
except ImportError as ex:
    print("  failed:")
    print("  Please add /your/CVAC_dir/3rdparty/ICE/python to PYTHONPATH.")
    print("  Detailed error message: {0}".format( ex ));

print("Trying to import opencv... ")
try:
    import cv
    print("  succeeded.")
except ImportError as ex:
    print("  failed:")
    print("  Please add /your/CVAC_dir/3rdparty/opencv/lib/python2.6/site-packages to PYTHONPATH.")
    print("  Detailed error message: {0}".format( ex ));

print("Trying to import cvac... ")
try:
    import cvac
    print("  succeeded.")
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
except ImportError as ex:
    print("  failed:")
    print("  Please add the directory in which easy.py is located to your PYTHONPATH.")
    print("  That's either /your/CVAC_dir/lib/python or /your/CVAC_dir/src/easy.")
    print("  Detailed error message: {0}".format( ex ));
