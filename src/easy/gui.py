
import sys
import os
import inspect
import subprocess
import threading
import time
from subprocess import PIPE
from StringIO import StringIO
import ConfigParser
import easy
import contextlib
import Queue

import Tkinter as tk

# Save stdout since redirecting to text widget
oldstdout = sys.stdout

# Add demos to sys path so import preqrequistes will work
scriptfname = inspect.getfile(inspect.currentframe())
scriptpath  = os.path.dirname(os.path.abspath( scriptfname ))
installpath = os.path.abspath(scriptpath+'/../..')
    
#get path to the python running this script
pythonExec = sys.executable

sys.path.append(installpath+'/python/easy')
sys.path.append(installpath+'/demo')
sys.path.append(installpath+'/3rdparty/ICE/python')
sys.path.append(installpath+'/python')
sys.path.append(installpath+'/virt/lib/python2.7/site-packages')

'''Michael Lange <klappnase (at) freakmail (dot) de>
The ToolTip class provides a flexible tooltip widget for Tkinter; it is based on IDLE's ToolTip
module which unfortunately seems to be broken (at least the version I saw).
INITIALIZATION OPTIONS:
anchor :        where the text should be positioned inside the widget, must be on of "n", "s", "e", "w", "nw" and so on;
                default is "center"
bd :            borderwidth of the widget; default is 1 (NOTE: don't use "borderwidth" here)
bg :            background color to use for the widget; default is "lightyellow" (NOTE: don't use "background")
delay :         time in ms that it takes for the widget to appear on the screen when the mouse pointer has
                entered the parent widget; default is 1500
fg :            foreground (i.e. text) color to use; default is "black" (NOTE: don't use "foreground")
follow_mouse :  if set to 1 the tooltip will follow the mouse pointer instead of being displayed
                outside of the parent widget; this may be useful if you want to use tooltips for
                large widgets like listboxes or canvases; default is 0
font :          font to use for the widget; default is system specific
justify :       how multiple lines of text will be aligned, must be "left", "right" or "center"; default is "left"
padx :          extra space added to the left and right within the widget; default is 4
pady :          extra space above and below the text; default is 2
relief :        one of "flat", "ridge", "groove", "raised", "sunken" or "solid"; default is "solid"
state :         must be "normal" or "disabled"; if set to "disabled" the tooltip will not appear; default is "normal"
text :          the text that is displayed inside the widget
textvariable :  if set to an instance of Tkinter.StringVar() the variable's value will be used as text for the widget
width :         width of the widget; the default is 0, which means that "wraplength" will be used to limit the widgets width
wraplength :    limits the number of characters in each line; default is 150

WIDGET METHODS:
configure(**opts) : change one or more of the widget's options as described above; the changes will take effect the
                    next time the tooltip shows up; NOTE: follow_mouse cannot be changed after widget initialization

Other widget methods that might be useful if you want to subclass ToolTip:
enter() :           callback when the mouse pointer enters the parent widget
leave() :           called when the mouse pointer leaves the parent widget
motion() :          is called when the mouse pointer moves inside the parent widget if follow_mouse is set to 1 and the
                    tooltip has shown up to continually update the coordinates of the tooltip window
coords() :          calculates the screen coordinates of the tooltip window
create_contents() : creates the contents of the tooltip window (by default a Tkinter.Label)
'''
# Ideas gleaned from PySol

import Tkinter

class ToolTip:
    def __init__(self, master, text='Your text here', delay=1500, **opts):
        self.master = master
        self._opts = {'anchor':'center', 'bd':1, 'bg':'lightyellow', 'delay':delay, 'fg':'black',\
                      'follow_mouse':0, 'font':None, 'justify':'left', 'padx':4, 'pady':2,\
                      'relief':'solid', 'state':'normal', 'text':text, 'textvariable':None,\
                      'width':0, 'wraplength':150}
        self.configure(**opts)
        self._tipwindow = None
        self._id = None
        self._id1 = self.master.bind("<Enter>", self.enter, '+')
        self._id2 = self.master.bind("<Leave>", self.leave, '+')
        self._id3 = self.master.bind("<ButtonPress>", self.leave, '+')
        self._follow_mouse = 0
        if self._opts['follow_mouse']:
            self._id4 = self.master.bind("<Motion>", self.motion, '+')
            self._follow_mouse = 1
    
    def configure(self, **opts):
        for key in opts:
            if self._opts.has_key(key):
                self._opts[key] = opts[key]
            else:
                KeyError = 'KeyError: Unknown option: "%s"' %key
                raise KeyError
    
    ##----these methods handle the callbacks on "<Enter>", "<Leave>" and "<Motion>"---------------##
    ##----events on the parent widget; override them if you want to change the widget's behavior--##
    
    def enter(self, event=None):
        self._schedule()
        
    def leave(self, event=None):
        self._unschedule()
        self._hide()
    
    def motion(self, event=None):
        if self._tipwindow and self._follow_mouse:
            x, y = self.coords()
            self._tipwindow.wm_geometry("+%d+%d" % (x, y))
    
    ##------the methods that do the work:---------------------------------------------------------##
    
    def _schedule(self):
        self._unschedule()
        if self._opts['state'] == 'disabled':
            return
        self._id = self.master.after(self._opts['delay'], self._show)

    def _unschedule(self):
        id = self._id
        self._id = None
        if id:
            self.master.after_cancel(id)

    def _show(self):
        if self._opts['state'] == 'disabled':
            self._unschedule()
            return
        if not self._tipwindow:
            self._tipwindow = tw = Tkinter.Toplevel(self.master)
            # hide the window until we know the geometry
            tw.withdraw()
            tw.wm_overrideredirect(1)

            if tw.tk.call("tk", "windowingsystem") == 'aqua':
                tw.tk.call("::tk::unsupported::MacWindowStyle", "style", tw._w, "help", "none")

            self.create_contents()
            tw.update_idletasks()
            x, y = self.coords()
            tw.wm_geometry("+%d+%d" % (x, y))
            tw.deiconify()
    
    def _hide(self):
        tw = self._tipwindow
        self._tipwindow = None
        if tw:
            tw.destroy()
                
    ##----these methods might be overridden in derived classes:----------------------------------##
    
    def coords(self):
        # The tip window must be completely outside the master widget;
        # otherwise when the mouse enters the tip window we get
        # a leave event and it disappears, and then we get an enter
        # event and it reappears, and so on forever :-(
        # or we take care that the mouse pointer is always outside the tipwindow :-)
        tw = self._tipwindow
        twx, twy = tw.winfo_reqwidth(), tw.winfo_reqheight()
        w, h = tw.winfo_screenwidth(), tw.winfo_screenheight()
        # calculate the y coordinate:
        if self._follow_mouse:
            y = tw.winfo_pointery() + 20
            # make sure the tipwindow is never outside the screen:
            if y + twy > h:
                y = y - twy - 30
        else:
            y = self.master.winfo_rooty() + self.master.winfo_height() + 3
            if y + twy > h:
                y = self.master.winfo_rooty() - twy - 3
        # we can use the same x coord in both cases:
        x = tw.winfo_pointerx() - twx / 2
        if x < 0:
            x = 0
        elif x + twx > w:
            x = w - twx
        return x, y

    def create_contents(self):
        opts = self._opts.copy()
        for opt in ('delay', 'follow_mouse', 'state'):
            del opts[opt]
        label = Tkinter.Label(self._tipwindow, **opts)
        label.pack()


'''
This thead class outputs the commands stdout and stderr to our display.
This is used to get the output from the servers.
'''
class ThreadWait(threading.Thread):
    def __init__(self, command, args, shell, env):
        threading.Thread.__init__(self)
        self.command = command
        self.args = args
        self.shell = shell
        self.env = env
        self.daemon = True  # Don't wait for this thead to end
        # Unix, Windows and old Macintosh end-of-line
        self.newlines = ['\n', '\r\n', '\r']

    def unbuffered(self, proc, stream='stdout'):
        stream = getattr(proc, stream)
        with contextlib.closing(stream):
            while True:
                out = []
                last = stream.read(1)
                # Don't loop forever
                if last == '' and proc.poll() is not None:
                    break
                while last not in self.newlines:
                    # Don't loop forever
                    if last == '' and proc.poll() is not None:
                        break
                    out.append(last)
                    last = stream.read(1)
                out = ''.join(out)
                yield out

    def run(self):
        try:
            p = subprocess.Popen([self.command, self.args], shell=self.shell, 
                             stdout=PIPE, stderr=subprocess.STDOUT, 
                             env=self.env)
            for line in self.unbuffered(p):
                # We can't use print line since this is a different thread
                # so we put it in a queue to be read later
                #print line
                gui_stdout_queue.put(line)
        except Exception, err:
            print("Could not run: {0}".format(self.command))
            print(err)


class Application(tk.Frame):

    #Since our application might not have a stdout sets redirect it to our text widget
    class StdoutRedirector(object):
        def __init__(self, text_widget, root):
            self.text_space = text_widget
            self.root = root

        def write(self, str):
            self.text_space.insert('end', str)
            self.text_space.see('end')
            #force write of widget
            self.root.update_idletasks()

    # Execute the command and optional arguments.
    # Use these envirnoment variables
    # set shell = true if a shell script with "#! shell"
    # if dopipe then wait for results and output to our stdout
    # if dothread then fork a thead that prints stdout/stderr to our window
    # env is dictionary of env vars that need to be set
    def doExec(self, command, args=None, shell=False, dopipe=False, 
               dothread = False, env=None):
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
            if dothread == True:
                mythread = ThreadWait(command, args, shell, env)
                mythread.start()
            else:
                try:
                    pipe = subprocess.Popen([command, args], shell=shell, 
                                    stdout=PIPE, stderr=PIPE, env=env)
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
        self.stdPollTime = 500  # number of msecs to poll stdout from thead
        self.checkVar = tk.IntVar() # 1 if checked, 0 otherwise
        if sys.platform=='win32':
            self.checkVar.set(0)
        else:
            self.checkVar.set(1)
        row = 1
        row = self.uiStartStopServices(row)
        row = self.uiCommands(row)
        row = self.uiLastButtons(row)
        self.output = tk.Text(self, wrap='word', height=30, width=40)
        self.output.grid(row=row, columnspan=2, sticky='NSWE', padx=5, pady=5)
        sys.stdout = self.StdoutRedirector(self.output, self.root)
        self.updateServerStatus()
        self.after(self.stdPollTime, self.updateGuiStdout)
        if sys.platform=='darwin':
            self.env = {'PYTHONPATH':installpath+'/3rdparty/ICE/python:'+installpath+'/virt/lib/python2.7/site-packages', 'DYLD_LIBRARY_PATH': installpath+'/3rdparty/opencv/lib:'+installpath+'/3rdparty/ICE/lib', 'PATH':installpath+'/virt/bin:' + os.getenv('PATH')}
            self.ccenv = {'PATH':installpath + '/virt/bin:' + os.getenv('PATH'),
                          'DYLD_LIBRARY_PATH': installpath+'/3rdparty/opencv/lib:'+installpath+'/3rdparty/ICE/lib'}
        elif sys.platform=='win32':
            # Added SystemRoot since python 2.7 seems to require it
            self.env = {'PYTHONPATH':installpath+'/3rdparty/ICE/python;'+installpath+'/python',
                       'PATH':installpath + '/virt/Scripts;' + installpath+'/bin;'+installpath+'/3rdparty/opencv/x86/vc10/bin;'+installpath+'/3rdparty/ICE/bin;'+ os.getenv('PATH'),
                       'SystemRoot': os.getenv('SystemRoot')}
            self.ccenv = self.env

    '''
    Since the stdout data is in a different thread we write it
    in a thread safe manner here
    '''
    def updateGuiStdout(self):
        while not gui_stdout_queue.empty():
            text = gui_stdout_queue.get()
            print(text)
        self.after(self.stdPollTime, self.updateGuiStdout)

    def uiCommands(self, row):
        lf = tk.LabelFrame(self, text='Commands:')
        lf.grid(row=row, columnspan=5, sticky="ew")
        row = row + 1
        docButton = tk.Button(lf, text="Documentation", width=20,
                                            bg="light blue",
                                            command=self.openDoc)
        docButton.grid(row=0, padx=5, pady=5, sticky=tk.W)
       
        demoButton = tk.Button(lf, text='Detect Demo', width=20,
                                        bg="light blue",
                                        command=lambda: self.runDemo('demo/detect.py'))
        demoButton.grid(row=0, column=1, padx=5, pady=5)
        
        terminalButton = tk.Button(lf, text='Open Terminal Window', width=20,
                                        bg="light blue",
                                        command=self.openTerminal)
        terminalButton.grid(row=1, padx=5, pady=5)
        
        envButton = tk.Button(lf, text="Python Env", width=20,
                                            bg="light blue",
                                            command=self.openEnv)
        envButton.grid(row=1, column=1, padx=5, pady=5)
        
        prereqButton = tk.Button(lf, text="Prerequistes", width=20,
                                            bg="light blue",
                                            command=self.runPrerequisites)
        prereqButton.grid(row=2, padx=5, pady=5)
       
        #self.commandStatus = tk.StringVar()
        #tk.Label(lf, text="Command Status:").grid(row=2, sticky=tk.W)

        #statusLabel = tk.Label(lf, textvariable=self.commandStatus)
        #statusLabel.grid(row=3, columnspan=5, sticky=tk.W)
  
        return row

    def runPrerequisites(self):
        if  sys.platform=='darwin':
            self.doExec(pythonExec, args=installpath + "/demo/prerequisites.py", dopipe=True, 
                    env=self.env)
        elif sys.platform=='win32':
            self.doExec(pythonExec, args=installpath + "/demo/prerequisites.py", dopipe=True, 
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
        self.dataDir = easy.getFSPath("", abspath=True)
        row = row + 1
        self.startButton = tk.Button(lf, text='start services', 
                                 bg="light blue", width=12,
                                 command=lambda: self.startStopServices(True))
        self.startButton.grid(row=0, sticky=tk.W, padx=5)
        self.stopButton = tk.Button(lf, text='stop services', 
                                 bg="light blue", width=12,
                                 command=lambda: self.startStopServices(False))
        self.stopButton.grid(row=0, column=1, padx=5)
        self.statusButton = tk.Button(lf, text='status', 
                                 bg="light blue", width=12,
                                 command=lambda: self.runServerStatus())
        self.statusButton.grid(row=0, column=2, padx=5, sticky=tk.E)
        tk.Label(lf, text="Service Status:").grid(row=2, sticky=tk.W)
        statusLabel = tk.Label(lf, textvariable=self.serverStatus)
        statusLabel.grid(row=2, column=1, pady=5, columnspan=4, sticky=tk.W)
        tk.Label(lf, text="Data Dir:").grid(row=3, sticky=tk.W)
        dirText = tk.Text(lf, width=38, height=1)
        dirText.insert(tk.END, self.dataDir)
        dirText.configure(state=tk.DISABLED, relief='groove', wrap='char',
                          borderwidth=2)
        dirText.grid(row=3, column=1, columnspan=4, sticky=tk.W)
        toolTip = ToolTip(dirText, text=self.dataDir, wraplength=600, 
                          delay=1000)
        if sys.platform!='win32':
            checkButton = tk.Checkbutton(lf, text= 'Show services output',
                                          variable=self.checkVar)
            checkButton.grid(row=5, columnspan=5, sticky=tk.W)
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
            if self.checkVar.get() == 1:
                setdothread = True
            else:
                setdothread = False
            if start:
                try:
                    self.doExec("/bin/bash", args=installpath + "/bin/startServices.sh", dothread = setdothread, env=self.ccenv) 
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
            shellcmd = 'start cmd /K type {0}\\python_env.txt'.format(os.getcwd())
            os.system( shellcmd )
        else:
            print "please define openEnv for this OS: "+sys.platform

    def openDoc(self):
        if sys.platform=='darwin':
            os.system("open "+os.getcwd()+'/doc/index.html')
        elif sys.platform=='win32':
            shellcmd = 'start {0}\\doc\\index.html'.format(os.getcwd())
            os.system( shellcmd )
        else:
            print "please define openDoc for this OS: "+sys.platform
            
    def openTerminal(self):
        if sys.platform=='darwin':
            # a lovely command to get a Terminal window with proper PYTHONPATH set
            shellcmd = "osascript -e 'tell application \"Terminal\" to activate' -e 'tell application \"System Events\" to tell process \"Terminal\" to keystroke \"n\" using command down' -e 'tell application \"Terminal\" to do script \"export PYTHONPATH="+installpath+'/3rdparty/ICE/python:'+installpath+'/python'+ ';' + 'export PATH='+installpath+'/virt/bin:$PATH; export DYLD_LIBRARY_PATH='+installpath + '/3rdparty/opencv/lib:'+installpath+'/3rdparty/ICE/lib' +"\" in the front window'"
            os.system( shellcmd )
        elif sys.platform=='win32':
            shellcmd = 'start cmd /K "set PATH={0}/bin;{0}/virt/Scripts;{0}/3rdparty/opencv/x86/vc10/bin;{0}/3rdparty/ICE/bin;%PATH%"'.format(installpath)
            os.system( shellcmd )
        else:
            print "please define openTerminal command for this OS: "+sys.platform
    
    def runDemo(self, demo):
        if sys.platform=='darwin':
            # a lovely command to get a Terminal window with proper PYTHONPATH set
            self.doExec(pythonExec, args=installpath + "/demo/detect.py", dopipe=True, env=self.env)
        elif sys.platform=='win32':
            shellcmd = 'start cmd /K "set PATH={0}/bin;{0}/3rdparty/opencv/x86/vc10/bin;{0}/3rdparty/ICE/bin;%PATH% && {2} {1}"'.format(installpath, demo, pythonExec)
            os.system( shellcmd )
        else:
            print "please define openTerminal command for this OS: "+sys.platform
        
    def getProxies(self):
        ''' Fetch all the proxies in the config.client file and return dictionary of
            {name:proxystring}
        '''
        config = StringIO()
        config.write('[dummysection]\n')
        res = {}
        try:
            f = open('config.client')
            config.write(f.read())
            config.seek(0)
            configParser = ConfigParser.RawConfigParser()
            # key option names case sensitive
            configParser.optionxform = str
            configParser.readfp(config)
            items = configParser.items('dummysection')
            for entry in items:
                if entry[0].endswith('.Proxy'):
                    name = entry[0].split('.Proxy',1)
                    if not res.has_key(name[0]):
                        res[name[0]] = entry[1]
            # Remove duplicate values
            inv = {}
            for k, v in res.iteritems():
                if not inv.has_key(k):
                    inv[v] = k
            res = {}
            for k, v in inv.iteritems():
                res[v] = k
                   
        except Exception, err:
            print("Could not get proxies from config.client")
            print(err)
         
        return res
    
    def verifyProxies(self, proxies):
        ''' Take as input dictionary of {name:proxystring} and verify that
            we can communicate with the server (by getting the detector or trainer).
            Return a dictionary with 'detector running' or 'trainer running' if running 
            and 'configured' if not. Also print now so with long config files user
            sees output.
        '''
        res = {}
        for key, value in proxies.iteritems():
            # add short timeout
            value = value + ' -t 100'
            try:
                detector = easy.getDetector(value)
                res[key] = 'detector running'
                print("{0} {1}".format(key, res[key]))
                continue
            except:
                pass
            try:
                trainer = easy.getTrainer(value)
                res[key] = 'trainer running'
                print("{0} {1}".format(key, res[key]))
                continue
            except:
                pass
            try:
                trainer = easy.getFileServer(value)
                res[key] = 'FileServer running'
                print("{0} {1}".format(key, res[key]))
                continue
            except:
                pass
            try:
                trainer = easy.getCorpusServer(value)
                res[key] = 'Corpus running'
                print("{0} {1}".format(key, res[key]))
                continue
            except:
                pass
            res[key] = 'not running'
            print("{0} {1}".format(key, res[key]))
        return res
    
    def runServerStatus(self):
        print("Server Status")
        proxies = self.getProxies()
        if len(proxies) > 0:
            self.verifyProxies(proxies)
        print("Server Status complete")  
    
       
            
       
root = Tkinter.Tk()
root.geometry('420x755+10+10')
root.tk_setPalette(background='light grey')
gui_stdout_queue = Queue.Queue()
os.chdir(installpath)
app = Application( master=root )
print("Running Prerequisites")
app.runPrerequisites()
app.master.title('EasyCV Control Center')
app.mainloop()
sys.stdout = oldstdout

