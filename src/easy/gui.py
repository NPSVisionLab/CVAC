import sys
import os
import inspect
import subprocess
import time
from subprocess import PIPE
from StringIO import StringIO

import Tkinter as tk

# Save stdout since redirecting to text widget
oldstdout = sys.stdout

# Add demos to sys path so import preqrequistes will work
scriptfname = inspect.getfile(inspect.currentframe())
scriptpath  = os.path.dirname(os.path.abspath( scriptfname ))
if sys.platform=='darwin':
    installpath = os.path.abspath(scriptpath+'/../Resources')
elif sys.platform=='win32':
    installpath = os.path.abspath(scriptpath+'/../..')
    

sys.path.append(installpath+'/python/easy')
sys.path.append(installpath+'/demo')
sys.path.append(installpath+'/3rdparty/ICE/python')
sys.path.append(installpath+'/python')


class Application(tk.Frame):

    #Since our application might not have a stdout sets redirect it to our text widget
    class StdoutRedirector(object):
        def __init__(self, text_widget):
            self.text_space = text_widget

        def write(self, str):
            self.text_space.insert('end', str)
            self.text_space.see('end')

    # Execute the command and optional arguments.
    # Use these envirnoment variables
    # set shell = true if a shell script with "#! shell"
    # if dopipe then wait for results and output to our stdout
    # env is dictionary of env vars that need to be set
    def doExec(self, command, args=None, shell=False, dopipe=False, env=None):
        if args == None:
            try:
                pipe = subprocess.Popen([command], shell=shell, stdout=PIPE, stderr=PIPE, env=env)
                if dopipe == True:
                    #This will hang until process finishes
                    outstr, errstr = pipe.communicate()
                    print(outstr)
                    print(errstr)
            except Exception, err:
                print("Could not run: {0}".format(command))
                print(err)
        else:
            try:
                pipe = subprocess.Popen([command, args], shell=shell, stdout=PIPE, stderr=PIPE, env=env)
                if dopipe == True:
                    outstr, errstr = pipe.communicate()
                    print(outstr)
                    print(errstr)
            except Exception, err:
                print("Could not run: {0}".format(command))
                print(err)
      
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.root = master;
        self.columnconfigure(1, minsize=295)
        self.grid()
        row = 1
        row = self.uiStartStopServices(row)
        row = self.uiCommands(row)
        row = self.uiLastButtons(row)
        self.output = tk.Text(self, wrap='word', height=30, width=40)
        self.output.grid(row=row, columnspan=2, sticky='NSWE', padx=5, pady=5)
        sys.stdout = self.StdoutRedirector(self.output)
        self.updateServerStatus()
        if sys.platform=='darwin':
            self.env = {'PYTHONPATH':installpath+'/3rdparty/ICE/python:'+installpath+'/python'}
        elif sys.platform=='win32':
            self.env = {'PYTHONPATH':installpath+'/3rdparty/ICE/python;'+installpath+'/python',
            'PATH':installpath+'/bin;'+installpath+'/3rdparty/opencv/bin;'+installpath+'/3rdparty/ICE/bin;%PATH%'}

    def uiCommands(self, row):
        lf = tk.LabelFrame(self, text='Commands:')
        lf.grid(row=row, columnspan=5, sticky="ew")
        row = row + 1
        terminalButton = tk.Button(lf, text='Open Terminal Window', width=20,
                                        bg="light blue",
                                        command=self.openTerminal)
        terminalButton.grid(row=0, padx=5, pady=5, sticky=tk.W)
        demoButton = tk.Button(lf, text='Detect Demo', width=20,
                                        bg="light blue",
                                        command=lambda: self.runDemo('demo/detect.py'))
        demoButton.grid(row=0, column=1, padx=5, pady=5)
        
        prereqButton = tk.Button(lf, text="Prerequistes", width=20,
                                            bg="light blue",
                                            command=self.runPrerequisites)
        prereqButton.grid(row=1, padx=5, pady=5)
        envButton = tk.Button(lf, text="Python Env", width=20,
                                            bg="light blue",
                                            command=self.openEnv)
        envButton.grid(row=1, column=1, padx=5, pady=5)
        #self.commandStatus = tk.StringVar()
        #tk.Label(lf, text="Command Status:").grid(row=2, sticky=tk.W)

        #statusLabel = tk.Label(lf, textvariable=self.commandStatus)
        #statusLabel.grid(row=3, columnspan=5, sticky=tk.W)
  
        return row

    def runPrerequisites(self):
        if  sys.platform=='darwin':
            self.doExec("/usr/bin/python2.6", args=installpath + "/demo/prerequisites.py", dopipe=True, 
                    env=self.env)
        elif sys.platform=='win32':
            self.doExec("/python26/python", args=installpath + "/demo/prerequisites.py", dopipe=True, 
                    env=self.env)
   
    def updateServerStatus(self):
        lockFileExists = os.path.isfile(installpath + '/.services_started.lock')
        if lockFileExists:
            self.serverStatus.set(
                    'services have been started (lock file exists)')
        else:
            self.serverStatus.set(
                     'services have not been started (no lock file)')

    def uiStartStopServices(self, row):
        lf = tk.LabelFrame(self, text='Control CVAC Services:')
        lf.grid(row=row, columnspan=5, sticky="ew")
        self.serverStatus = tk.StringVar()
        row = row + 1
        self.startButton = tk.Button(lf, text='start services', 
                                 bg="light blue",
                                 command=lambda: self.startStopServices(True))
        self.startButton.grid(row=0, sticky=tk.W, padx=5)
        self.stopButton = tk.Button(lf, text='stop services', 
                                 bg="light blue",
                                 command=lambda: self.startStopServices(False))
        self.stopButton.grid(row=0, column=1, pady=5)
        tk.Label(lf, text="Service Status:").grid(row=2, sticky=tk.W)
        statusLabel = tk.Label(lf, textvariable=self.serverStatus)
        statusLabel.grid(row=3, columnspan=5, sticky=tk.W)
        return row
    
    def uiLastButtons(self, row):
        f = tk.Frame(self, pady=5)
        f.grid(row=row, columnspan=5, sticky="ew")
        row = row + 1
        quitButton = tk.Button(f, text='Quit', bg="light blue", command=self.quit)
        quitButton.pack(side=tk.RIGHT)
        return row

    def startStopServices(self, start):
        print("StartStopServices called start = {0}".format(start))
        if sys.platform=='darwin':
            if start:
                try:
                    self.doExec("/bin/bash", args=installpath + "/bin/startServices.sh") 
                    #self.doExec(installpath+'/bin/startServices.sh', shell=False)
                except Exception, err:
                    print ("Could not start/stop services")
                    print err
                #os.system(os.getcwd()+'/bin/startServices.sh')
            else:
                try:
                    self.doExec("/bin/bash", args=installpath + "/bin/stopServices.sh") 
                except Exception, err:
                    print ("Could not start/stop services")
                    print err
        elif sys.platform=='win32':
            if start:
                os.system(installpath+'/bin/startServices.bat')
            else:
                os.system(installpath+'/bin/stopServices.bat')
        else:
            print "please define start/stop commands for this OS: "+sys.platform
        #give the services a chance to delete or create the lock file
        time.sleep(2)
        self.updateServerStatus()

    def openEnv(self):
        if sys.platform=='darwin':
            os.system("open "+os.getcwd()+'/python_env.txt')
        elif sys.platform=='win32':
            shellcmd = 'start cmd /K type {0}\\demo\\python_env.txt'.format(os.getcwd())
            os.system( shellcmd )
        else:
            print "please define openEnv for this OS: "+sys.platform

    def openTerminal(self):
        if sys.platform=='darwin':
            # a lovely command to get a Terminal window with proper PYTHONPATH set
            shellcmd = "osascript -e 'tell application \"Terminal\" to activate' -e 'tell application \"System Events\" to tell process \"Terminal\" to keystroke \"n\" using command down' -e 'tell application \"Terminal\" to do script \"export PYTHONPATH="+installpath+'/3rdparty/ICE/python:'+installpath+'/python'+"\" in the front window'"
            os.system( shellcmd )
        elif sys.platform=='win32':
            shellcmd = 'start cmd /K "set PATH={0}/bin;{0}/3rdparty/opencv/bin;{0}/3rdparty/ICE/bin;%PATH%"'.format(installpath)
            os.system( shellcmd )
        else:
            print "please define openTerminal command for this OS: "+sys.platform
    
    def runDemo(self, demo):
        if sys.platform=='darwin':
            # a lovely command to get a Terminal window with proper PYTHONPATH set
            self.doExec("/usr/bin/python2.6", args=installpath + "/demo/detect.py", dopipe=True, env=self.env)
        elif sys.platform=='win32':
            shellcmd = 'start cmd /K "set PATH={0}/bin;{0}/3rdparty/opencv/bin;{0}/3rdparty/ICE/bin;%PATH% && \\python26\\python.exe {1}"'.format(installpath, demo)
            os.system( shellcmd )
        else:
            print "please define openTerminal command for this OS: "+sys.platform
        
       
root = tk.Tk()
root.geometry('410x720+10+10')
root.tk_setPalette(background='light grey')
os.chdir(installpath)
app = Application( master=root )
print("Running Prerequisites")
app.runPrerequisites()
app.master.title('EasyCV Control Center')
app.mainloop()
sys.stdout = oldstdout
root.destroy()
