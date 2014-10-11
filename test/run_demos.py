'''
Test to verify that a binary install worked correctly by running some
demos.
'''

import sys
import os
import time
import platform
from subprocess import call


if __name__ == '__main__' :
    try:
        platform = platform.system()
        extension = '.sh'
        if platform == 'Windows':
            extension = '.bat'
        thisPath = os.path.dirname(os.path.abspath(__file__))
        binPath = os.path.abspath(thisPath + "/../bin")
        demoPath = os.path.abspath(thisPath + "/../demo")
        pathenv = os.getenv("PATH")
        if platform == 'Windows':
            os.putenv("PATH", pathenv + ";" + thisPath + "/../3rdparty/Ice/bin")
        else:
            os.putenv("PATH", pathenv + ":" + thisPath + "/../3rdparty/Ice/lib")
        # Need to stop any old services incase we had a crash last time
        # This gets rid of the lock file incase its still there
        call([binPath + "/stopServices" + extension], shell=True)
        time.sleep(2)
        print("Starting " + binPath + "/startServices" + extension)
        call([binPath + "/startServices" + extension], shell=True)
        time.sleep(8)
        execfile(demoPath + '/prerequisites.py')
        execfile(demoPath + '/detect.py')
        execfile(demoPath + '/training.py')
        execfile(demoPath + '/runset.py')
        execfile(demoPath + '/full_image_corpus.py')
        execfile(demoPath + '/bootstrapping.py')
        time.sleep(5)
        try:
            call([binPath + "/stopServices" + extension], shell=True)
        except:
            pass
    except:
        # If we get an error or forced quit we still want to stop the services
        try:
            call([binPath + "/stopServices" + extension], shell=True)
        except:
            pass


