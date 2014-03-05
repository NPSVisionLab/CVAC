import Tkinter as tk
import sys
import os

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.grid()
        self.determineConfiguration()
        self.createWidgets()

    def createWidgets(self):
        self.quitButton = tk.Button(self, text='Quit', command=self.quit)
        self.quitButton.grid()

    def determineConfiguration(self):
        sys.path.append('demo')
        import prerequisites
        print prerequisites.success
        #text = tk.Text()
        #text.insert(tk.END, 'hello')
        #text.pack(side='left')
        self.detailsButton = tk.Button(self, text='Show details', 
                                       command=self.openEnv)
        self.detailsButton.grid()

    def openEnv(self):
        os.system("open "+os.getcwd()+'/python_env.txt')
        
root = tk.Tk()
app = Application( master=root )
app.master.title('EasyCV Control Center')
app.mainloop()
root.destroy()
