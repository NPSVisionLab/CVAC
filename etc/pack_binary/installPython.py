import Tkinter as tk
import sys
import os
import subprocess
import inspect

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.root = master
        self.grid()
        label = tk.Label(self, text = "EasyCV requires Python 2.7")
        label.grid(row=0, column=0, padx=5, pady=5, sticky="w")
        label = tk.Label(self, text = "Install Python 2.7?")
        label.grid(row=1, column=0, padx=5, pady=5, sticky="w")
        f = tk.Frame(self, pady=5)
        f.grid(row=2)
        okbutton = tk.Button(f, text="Yes", bg="light blue", width=6,
                           command=self.runOK)
        okbutton.grid(row=0, column=0, padx=5, pady=5)
        quitbutton = tk.Button(f, text="No", bg="light blue", width=6,
                           command=self.quit)
        quitbutton.grid(row=0, column=1, padx=5, pady=5)
        plabel = tk.Label(self, text = "Enter password to install.")
        plabel.grid(row=3, column=0, padx=5, pady=5, sticky="w")
        self.passwrd = tk.Entry(self, bd = 3, show="*")
        self.passwrd.grid(row=4, column=0, padx=5, pady=5, sticky="w")

    def runOK(self):
        # Determine path to 3rdparty directory
        scriptfname = inspect.getfile(inspect.currentframe())
        scriptpath  = os.path.dirname(os.path.abspath( scriptfname ))
        installpath = os.path.abspath(scriptpath+'/../Resources')
        passwd = self.passwrd.get()
        p = subprocess.Popen(['sudo', 'installer', '-pkg',
                          installpath + '/3rdparty/Python.mpkg',
                          '-target', '/'], 
                          stdin=subprocess.PIPE,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
        print(p.communicate(passwd + "\n"))
        self.root.destroy()

root = tk.Tk()
root.geometry('190x190+30+30')
root.tk_setPalette(background='light grey')
app = Application( master=root )
app.master.title('EasyCV Install Python')
app.mainloop()
