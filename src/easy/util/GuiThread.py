'''
This thead class outputs the commands stdout and stderr to our display.
This is used to get the output from the servers.
'''
from future import standard_library
standard_library.install_aliases()
try:
    #lets try python 2.x style
    import tkinter as tk
except Exception as exc:
    #lets try python 3.x style
    import tkinter as tk
import threading
import sys
import time
import queue
import sys
from PIL import Image, ImageTk, ImageDraw



class GuiThread(threading.Thread):
    def __init__(self, queue):
        threading.Thread.__init__(self)
        self.canvas = None
        self.gui_queue = queue
        self.pollTime = 100
        self.defaultWindow = False
        self.windows = {}
        
    def getCanvas(self):
        return self.canvas
    
    def getQueue(self):
        return self.qui_queue;
    
    def processQueue(self):
        #import pydevd
        #pydevd.connected = True
        #pydevd.settrace(suspend=False)
        while not self.gui_queue.empty():
            cmd = self.gui_queue.get()
            text = cmd[0]
            #print("Gui command " + text)
            if text == "ImageWindow":
                img = cmd[1]
                wid = cmd[2]
                if wid == 0:
                    if self.defaultWindow == False:
                        self.newWindow(img, wid)
                    else:
                        self.updateWindow(img, wid)
                else:
                    if wid in list(self.windows.keys()):
                        self.updateWindow(img, wid)
                    else:
                        self.newWindow(img, wid)
                self.gui_queue.task_done()
            elif text == "CloseWindow":
                wid = cmd[1]
                self.closeWindow(wid)
                self.gui_queue.task_done()
            else:
                RuntimeError("command not supported - " + text)
            
        self.canvas.after(self.pollTime, self.processQueue)
       
        
    def newWindow(self, img, wid):
        if wid == 0:
            self.canvas.deiconify()
            self.canvas.title("CVAC Main Window")
            win = self.canvas
        else:
            win = tk.Toplevel()
            win.title("CVAC Window")
        photo = ImageTk.PhotoImage( img )
        
        # make the window the size of the image
        # position coordinates of wnd 'upper left corner'
        x = wid * 50
        y = wid * 50
        w = photo.width()
        h = photo.height()
        win.geometry("%dx%d+%d+%d" % (w, h, x, y))
        #wnd.create_image(0, 0, anchor= tk.NW, image=photo)
        #wnd.pack()
      
        # Display the photo in a label (as wnd has no image argument)
        panel = tk.Label(win, image=photo)
        panel.pack(side='top', fill='both', expand='yes')    
        # save the panel's image from 'garbage collection'
        panel.image = photo
        
        win.update()
        if wid > 0:
            self.windows[wid] = (win,panel)
        else:
            self.panel = panel
            self.defaultWindow = True

        
    def updateWindow(self, img, wid):
        if wid == 0:
            panel = self.panel
            win = self.canvas
        else:
            win = self.windows[wid][0]
            panel = self.windows[wid][1]
   
        photo = ImageTk.PhotoImage( img )
        panel.configure(image=photo)
        panel.image = photo
       
        win.update()
        
    def closeWindow(self, wid):
        
        if wid == 0:
            win = self.canvas
            self.defaultWindow = False
            win.destroy()
        else:
            win = self.windows[wid][0]
            self.windows.pop(wid)
            win.destroy()


    def run(self):
        #import pydevd
        #pydevd.connected = True
        #pydevd.settrace(suspend=False)
        from PIL import Image, ImageTk, ImageDraw
        self.canvas = tk.Tk()
        self.canvas.iconify()
        self.canvas.after(self.pollTime, self.processQueue)
        self.canvas.mainloop()

        

#Use main for test
if __name__ == '__main__':
    import time
    queue = queue.Queue()
    guithread = GuiThread(queue);
    img = Image.new("RGB", (300,300), "white")
    draw = ImageDraw.Draw(img)
    draw.ellipse((0, 0, 100, 100), fill="black")
    queue.put(("ImageWindow", img, 0))
    img2 = Image.new("RGB", (300,300), "black")
    draw2 = ImageDraw.Draw(img2)
    draw2.ellipse((0, 0, 200, 200), fill="red")
    queue.put(("ImageWindow", img2, 1))
    # For OSX the guithread must be the main one
    guithread = GuiThread(queue);
    guithread.run()
