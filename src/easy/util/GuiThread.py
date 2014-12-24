'''
This thead class outputs the commands stdout and stderr to our display.
This is used to get the output from the servers.
'''

import Tkinter as tk
import threading
from PIL import Image, ImageTk, ImageDraw
import Queue



class GuiThread(threading.Thread):
    def __init__(self, queue):
        threading.Thread.__init__(self)
        self.canvas = None
        self.gui_queue = queue
        self.pollTime = 100
        self.imageWindow = False
        
    def getCanvas(self):
        return self.canvas
    
    def getQueue(self):
        return self.qui_queue;
    
    def processQueue(self):
        while not self.gui_queue.empty():
            cmd = self.gui_queue.get()
            text = cmd[0]
            print("got queue " + text)
            if text == "ImageWindow":
                img = cmd[1]
                if self.imageWindow == False:
                    self.newWindow(img)
                else:
                    self.updateWindow(img)
                self.gui_queue.task_done()
            
        self.canvas.after(self.pollTime, self.processQueue)
       
        
    def newWindow(self, img):
       
        self.canvas.title("CVAC Results")
        self.canvas.deiconify()
        photo = ImageTk.PhotoImage( img )
        
        # make the window the size of the image
        # position coordinates of wnd 'upper left corner'
        x = 0
        y = 0
        w = photo.width()
        h = photo.height()
        self.canvas.geometry("%dx%d+%d+%d" % (w, h, x, y))
        #wnd.create_image(0, 0, anchor= tk.NW, image=photo)
        #wnd.pack()
      
        # Display the photo in a label (as wnd has no image argument)
        self.panel = tk.Label(self.canvas, image=photo)
        self.panel.pack(side='top', fill='both', expand='yes')    
        # save the panel's image from 'garbage collection'
        self.panel.image = photo
        self.imageWindow = True
        self.canvas.update()

        
    def updateWindow(self, img):
        photo = ImageTk.PhotoImage( img )
        self.panel.configure(image=photo)
        self.panel.image = photo
       
        self.canvas.update()

  
    def run(self):
        #import pydevd
        #pydevd.connected = True
        #pydevd.settrace(suspend=False)
        self.canvas = tk.Tk()
        self.canvas.iconify()
        self.canvas.after(self.pollTime, self.processQueue)
        self.canvas.mainloop()
        