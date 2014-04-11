import Tkinter as tk
import sys
import os

# Add demos to sys path so import preqrequistes will work
thisPath = os.getcwd()
demoPath = os.path.abspath(thisPath + '/demo')
sys.path.append(demoPath)

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.root = master;
        self.columnconfigure(1, minsize=295)
        self.grid()
        row = 1
        row = self.uiStartStopServices(row)
        row = self.uiCommands(row)
        row = self.uiLastButtons(row)
        self.updateServerStatus()
        self.runPrerequisites()
        

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
        self.commandStatus = tk.StringVar()
        tk.Label(lf, text="Command Status:").grid(row=2, sticky=tk.W)

        statusLabel = tk.Label(lf, textvariable=self.commandStatus)
        statusLabel.grid(row=3, columnspan=5, sticky=tk.W)
  
        return row

    def runPrerequisites(self):
        import prerequisites
        if prerequisites.success:
            self.commandStatus.set('prerequisites test succeeded')
        else:
            self.commandStatus.set( 'prerequisites test failed')
   
    def updateServerStatus(self):
        lockFileExists = os.path.isfile('.services_started.lock')
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
        if sys.platform=='darwin':
            if start:
                os.system(os.getcwd()+'/bin/startServices.sh')
            else:
                os.system(os.getcwd()+'/bin/stopServices.sh')
        elif sys.platform=='win32':
            if start:
                os.system(os.getcwd()+'/bin/startServices.bat')
            else:
                os.system(os.getcwd()+'/bin/stopServices.bat')
        else:
            print "please define start/stop commands for this OS: "+sys.platform
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
        installpath = os.getcwd()
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
        installpath = os.getcwd()
        if sys.platform=='darwin':
            # a lovely command to get a Terminal window with proper PYTHONPATH set
            shellcmd = "osascript -e 'tell application \"Terminal\" to activate' -e 'tell application \"System Events\" to tell process \"Terminal\" to keystroke \"n\" using command down' -e 'tell application \"Terminal\" to do script \"export PYTHONPATH="+installpath+'/3rdparty/ICE/python:'+installpath+'/python'+"\" in the front window'"
            os.system( shellcmd )
        elif sys.platform=='win32':
            shellcmd = 'start cmd /K "set PATH={0}/bin;{0}/3rdparty/opencv/bin;{0}/3rdparty/ICE/bin;%PATH% && \\python26\\python.exe {1}"'.format(installpath, demo)
            os.system( shellcmd )
        else:
            print "please define openTerminal command for this OS: "+sys.platform
        
       
        
root = tk.Tk()
# window 300x300 10 pixels left and down from corner of the screen
root.geometry('330x280+10+10')
root.tk_setPalette(background='light grey')
app = Application( master=root )
app.master.title('EasyCV Control Center')
app.mainloop()
root.destroy()
