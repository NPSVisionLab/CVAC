import Tkinter as tk
import sys
import os

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.grid()
        self.uiPrerequisites()
        self.uiStartStopServices()
        self.uiMiscellaneous()

    def uiMiscellaneous(self):
        self.terminalButton = tk.Button(self, text='Open Terminal Window',
                                        command=self.openTerminal)
        self.terminalButton.grid()
        self.quitButton = tk.Button(self, text='Quit', command=self.quit)
        self.quitButton.grid()

    def uiPrerequisites(self):
        import prerequisites
        text = tk.Text()
        if prerequisites.success:
            text.insert(tk.END, 'prerequisites test succeeded')
        else:
            text.insert(tk.END, 'prerequisites test failed')
        text.grid()
        self.detailsButton = tk.Button(self, text='Show details', 
                                       command=lambda: self.openEnv)
        self.detailsButton.grid()

    def uiStartStopServices(self):
        lockFileExists = os.path.isfile('.services_started.lock')
        text = tk.Text()
        if lockFileExists:
            text.insert(tk.END, 'services have been started (lock file exists)')
        else:
            text.insert(tk.END, 'services have not been started (no lock file)')
        text.grid()
        self.startButton = tk.Button(self, text='start services', 
                                     command=lambda: self.startStopServices(True))
        self.startButton.grid()
        self.stopButton = tk.Button(self, text='stop services', 
                                     command=lambda: self.startStopServices(False))
        self.stopButton.grid()

    def startStopServices(self, start):
        if sys.platform=='darwin':
            if start:
                os.system(os.getcwd()+'/bin/startServices.sh')
            else:
                os.system(os.getcwd()+'/bin/stopServices.sh')
        else:
            print "please define start/stop commands for this OS: "+sys.platform

    def openEnv(self):
        if sys.platform=='darwin':
            os.system("open "+os.getcwd()+'/python_env.txt')
        else:
            print "please define openEnv for this OS: "+sys.platform

    def openTerminal(self):
        installpath = os.getcwd()
        if sys.platform=='darwin':
            # a lovely command to get a Terminal window with proper PYTHONPATH set
            shellcmd = "osascript -e 'tell application \"Terminal\" to activate' -e 'tell application \"System Events\" to tell process \"Terminal\" to keystroke \"n\" using command down' -e 'tell application \"Terminal\" to do script \"export PYTHONPATH="+installpath+'/3rdparty/ICE/python:'+installpath+'/python'+"\" in the front window'"
            os.system( shellcmd )
        else:
            print "please define openTerminal command for this OS: "+sys.platform
       
        
root = tk.Tk()
app = Application( master=root )
app.master.title('EasyCV Control Center')
app.mainloop()
root.destroy()
