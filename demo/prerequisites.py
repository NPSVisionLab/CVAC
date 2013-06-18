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

print("Trying to import cvac... ")
try:
    import cvac
    print("  succeeded.")
except ImportError as ex:
    print("  failed:")
    print("  Please add /your/CVAC_dir/lib/python to PYTHONPATH.")
    print("  Detailed error message: {0}".format( ex ));

print("Trying to import easy... ")
try:
    import easy
    print("  succeeded.")
except ImportError as ex:
    print("  failed:")
    print("  Please add /your/CVAC_dir/lib/python to PYTHONPATH.")
    print("  Detailed error message: {0}".format( ex ));
