'''
Test to verify that a binary install worked correctly by running some
demos.
'''

import sys
import os
import time
import platform
import subprocess


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
        subprocess.call([binPath + "/stopServices" + extension], shell=True)
        time.sleep(2)
        print("Starting " + binPath + "/startServices" + extension)
        subprocess.call([binPath + "/startServices" + extension], shell=True)
        time.sleep(8)
        finalRes = 0
        pobj = subprocess.Popen([sys.executable, demoPath + '/prerequisites.py'])
        res = pobj.communicate()
        if res == 1:
            print("prerequisites.py failed")
            finalRes = 1
        pobj = subprocess.Popen([sys.executable, demoPath + '/detect.py'])
        res = pobj.communicate()
        if res == 1:
            print("detect.py failed")
            finalRes = 1
        pobj = subprocess.Popen([sys.executable, demoPath + '/training.py'])
        res = pobj.communicate()
        if res == 1:
            print("training.py failed")
            finalRes = 1
        pobj = subprocess.Popen([sys.executable, demoPath + '/runset.py'])
        res = pobj.communicate()
        if res == 1:
            print("runset.py failed")
            finalRes = 1
        pobj = subprocess.Popen([sys.executable, demoPath + '/full_image_corpus.py'])
        res = pobj.communicate()
        if res == 1:
            print("full_image_corpus.py failed")
            finalRes = 1
        pobj = subprocess.Popen([sys.executable, demoPath + '/bootstrapping.py'])
        res = pobj.communicate()
        if res == 1:
            print("bootstrapping.py failed")
            finalRes = 1
        try:
            subprocess.call([binPath + "/stopServices" + extension], shell=True)
        except:
            pass
    except Exception as e:
        #failure so set error return
        print(e)
        finalRes = 1
        # If we get an error or forced quit we still want to stop the services
        try:
            subprocess.call([binPath + "/stopServices" + extension], shell=True)
        except:
            pass

    exit(finalRes)

