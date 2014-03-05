import Tkinter as tk
import sys
import os

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.grid()
        self.uiPrerequisites()
        self.uiStartStopServices()
        self.createWidgets()

    def createWidgets(self):
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
                                       command=self.openEnv)
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
                                     command=self.startStopServices(True))
        self.startButton.grid()
        self.stopButton = tk.Button(self, text='stop services', 
                                     command=self.startStopServices(False))
        self.stopButton.grid()

    def startStopServices(self, start):
        if start:
            os.system("open "+os.getcwd()+'/bin/startServices.sh')
        else:
            os.system("open "+os.getcwd()+'/bin/stopServices.sh')

    def openEnv(self):
        os.system("open "+os.getcwd()+'/python_env.txt')
        
root = tk.Tk()
app = Application( master=root )
app.master.title('EasyCV Control Center')
app.mainloop()
root.destroy()
