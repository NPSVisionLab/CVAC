#!/usr/bin/python2.6
import sys

# get this file's path:
import inspect, os
scriptfname = inspect.getfile(inspect.currentframe())
scriptpath  = os.path.dirname(os.path.abspath( scriptfname ))

sys.path.append(os.path.abspath(scriptpath+'/../Resources/python/easy'))
sys.path.append(os.path.abspath(scriptpath+'/../Resources/demo'))
sys.path.append(os.path.abspath(scriptpath+'/../Resources/3rdparty/ICE/python'))
sys.path.append(os.path.abspath(scriptpath+'/../Resources/python'))

# open the simple GUI that displays system information
# and permits startup of the services
import gui

