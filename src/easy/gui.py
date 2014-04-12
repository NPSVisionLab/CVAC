import sys
import os
import inspect


def appendLog(string):
   file = open('/Users/tomb/mylog.txt', 'a')
   file.write(string)
   file.write('\n')
   file.close()

appendLog("in gui")
import Tkinter as tk

oldstdout = sys.stdout

# Add demos to sys path so import preqrequistes will work
appendLog("before class")
scriptfname = inspect.getfile(inspect.currentframe())
scriptpath  = os.path.dirname(os.path.abspath( scriptfname ))
installpath = os.path.abspath(scriptpath+'/../Resources')

sys.path.append(installpath+'/python/easy')
sys.path.append(installpath+'/demo')
sys.path.append(installpath+'/3rdparty/ICE/python')
sys.path.append(installpath+'/python')


class Application(tk.Frame):

    class StdoutRedirector(object):
        def __init__(self, text_widget):
            self.text_space = text_widget

        def write(self, str):
            self.text_space.insert('end', str)
            self.text_space.see('end')

    def __init__(self, master=None):
        appendLog("init frame")
        tk.Frame.__init__(self, master)
        appendLog("init frame done")
        self.root = master;
        self.columnconfigure(1, minsize=295)
        self.grid()
        appendLog("grid done")
        row = 1
        row = self.uiStartStopServices(row)
        row = self.uiCommands(row)
        row = self.uiLastButtons(row)
        self.output = tk.Text(self, wrap='word', height=30, width=40)
        self.output.grid(row=row, columnspan=2, sticky='NSWE', padx=5, pady=5)
        sys.stdout = self.StdoutRedirector(self.output)
        print("Output from stdout")
        self.updateServerStatus()
        appendLog("running pre")
        #self.runPrerequisites()

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
        appendLog(installdir)
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
            shellcmd = "osascript -e 'tell application \"Terminal\" to activate' -e 'tell application \"System Events\" to tell process \"Terminal\" to keystroke \"n\" using command down' -e 'tell application \"Terminal\" to do script \"export PYTHONPATH="+installpath+'/3rdparty/ICE/python:'+installpath+'/python'+"\"  in the front window'"
            os.system( shellcmd )
        elif sys.platform=='win32':
            shellcmd = 'start cmd /K "set PATH={0}/bin;{0}/3rdparty/opencv/bin;{0}/3rdparty/ICE/bin;%PATH% && \\python26\\python.exe {1}"'.format(installpath, demo)
            os.system( shellcmd )
        else:
            print "please define openTerminal command for this OS: "+sys.platform
        
       
        
appendLog("constructing root")
root = tk.Tk()
appendLog(" root constructed")
# window 300x300 10 pixels left and down from corner of the screen
root.geometry('350x380+10+10')
root.tk_setPalette(background='light grey')
appendLog("ready to construct application" )
app = Application( master=root )
appendLog("application constructed" )
app.master.title('EasyCV Control Center')
appendLog("ready for mainloop")
app.mainloop()
appendLog("mainloop done")
sys.stdout = oldstdout
root.destroy()
