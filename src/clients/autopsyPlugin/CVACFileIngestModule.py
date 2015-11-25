import jarray
import inspect
from javax.swing import JCheckBox
from javax.swing import BoxLayout
from java.io import File
from java.lang import System
from java.util.logging import Level
from java.util.UUID import randomUUID
from java.lang import IllegalArgumentException
from org.sleuthkit.datamodel import SleuthkitCase
from org.sleuthkit.datamodel import AbstractFile
from org.sleuthkit.datamodel import ReadContentInputStream
from org.sleuthkit.datamodel import BlackboardArtifact
from org.sleuthkit.datamodel import BlackboardAttribute
from org.sleuthkit.datamodel import TskData
from org.sleuthkit.autopsy.ingest import IngestModule
from org.sleuthkit.autopsy.ingest.IngestModule import IngestModuleException
from org.sleuthkit.autopsy.ingest import DataSourceIngestModule
from org.sleuthkit.autopsy.ingest import FileIngestModule
from org.sleuthkit.autopsy.ingest import IngestModuleFactoryAdapter
from org.sleuthkit.autopsy.ingest import IngestModuleIngestJobSettings
from org.sleuthkit.autopsy.ingest import IngestModuleIngestJobSettingsPanel
from org.sleuthkit.autopsy.ingest import IngestMessage
from org.sleuthkit.autopsy.ingest import IngestServices
from org.sleuthkit.autopsy.ingest import ModuleDataEvent
from org.sleuthkit.autopsy.coreutils import Logger
from org.sleuthkit.autopsy.casemodule import Case
from org.sleuthkit.autopsy.casemodule.services import Services
from org.sleuthkit.autopsy.casemodule.services import FileManager
from org.sleuthkit.autopsy.datamodel import ContentUtils

import shutil

import os
import sys
thisPath = os.path.dirname(os.path.abspath(__file__))
cvacPath = os.path.abspath(thisPath + "/cvac")
sys.path.append(thisPath)
import Ice
import Ice.Util
#Note since DetectorPrxHelper is a static class jypthon does not want
#a reference to it but only the parent class!
import cvac
import cvac.Detector
import cvac.DetectorPrx

import cvac.DetectorCallbackHandler
import cvac.RunSet
import cvac.Purpose
import cvac.ImageSubstrate
import cvac.Label
import cvac.Labelable
import cvac.PurposedLabelableSeq
import cvac.DetectorProperties
import java.util.logging.Level
import DetectorCallbackHandlerI;

def myenum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    return type('Enum', (), enums)

# The list of detectors/algorithms  we will support.
# TODO: read this from a config file.
Detectors = myenum('BOW_FLAG', 'BOW_LOGO', 'CV_FACE', 'WIRE', 'SS')
DetectorNames = ["BOW Flag Detector",
                 "BOW LOGO Detector",
                 "CV Face Detector",
                 "WireDiagram Detector",
                 "ScreenShot Detector"
                ]
_logger = Logger.getLogger("EASYCV Logger")

def dolog(mess):
    _logger.logp(java.util.logging.Level.WARNING, None .__class__.__name__, inspect.stack()[1][3], mess)

dolog("init cvac")

# We want to do this only once.  Startup gets called for each thread
args = sys.argv
args.append('--Ice.Config=config.client')
ic = Ice.Util.initialize(args)
CVAC_DataDir = ic.getProperties().getProperty("CVAC.DataDir")
dolog("init complete")

# This class keeps track of which detector/algorithms the user selected
class CVACIngestModuleSettings(IngestModuleIngestJobSettings):
    serialVersionUID = 1L

    def __init__(self):

        self.flags = []
        for i in range(5):
            self.flags.append(False)
        # Default we will turn on the first detector so something gets ran
        self.flags[0] = True

    def getVersionNumber(self):
        return self.serialVersionUID

    def getFlags(self):
        return self.flags

    def setFlags(self,  flags):
        self.flags = flags

# UI shown for the user for each ingest job.
# Note!!!! Python Errors in this module do not get reported!
# If an error occurs the Ingest Modules panel is just not shown
# This includes python syntax errors so use dolog to isolate the problem
# This displays a checkbox for each detector.  When a new detector/algorithm
# has been added above then add it here.
class CVACIngestModuleSettingsPanel(IngestModuleIngestJobSettingsPanel):
    # Note settings is used by base class
    def __init__(self, settings):

        self.local_settings = settings
        # Add JPanel components here
        self.initComponents()
        self.customizeComponents()

    def checkBoxBOW_FLAG(self, event):
        if self.checkbox_BOW_FLAG.isSelected():
            flags = self.local_settings.getFlags()
            flags[Detectors.BOW_FLAG] = True
            self.local_settings.setFlags(flags)
        else:
            flags = self.local_settings.getFlags()
            flags[Detectors.BOW_FLAG] = False
            self.local_settings.setFlag(flags)
    def checkBoxBOW_LOGO(self, event):
        if self.checkbox_BOW_LOGO.isSelected():
            flags = self.local_settings.getFlags()
            flags[Detectors.BOW_LOGO] = True
            self.local_settings.setFlags(flags)
        else:
            flags = self.local_settings.getFlags()
            flags[Detectors.BOW_LOGO] = False
            self.local_settings.setFlag(flags)
    def checkBoxCV_FACE(self, event):
        if self.checkbox_CV_FACE.isSelected():
            flags = self.local_settings.getFlags()
            flags[Detectors.CV_FACE] = True
            self.local_settings.setFlags(flags)
        else:
            flags = self.local_settings.getFlags()
            flags[Detectors.CV_FACE] = False
            self.local_settings.setFlag(flags)
    def checkBoxWIRE(self, event):
        dolog("in check wire")
        if self.checkbox_WIRE.isSelected():
            flags = self.local_settings.getFlags()
            dolog("setting wire to true")
            flags[Detectors.WIRE] = True
            self.local_settings.setFlags(flags)
        else:
            flags = self.local_settings.getFlags()
            flags[Detectors.WIRE] = False
            self.local_settings.setFlag(flags)
    def checkBoxSS(self, event):
        if self.checkbox_SS.isSelected():
            flags = self.local_settings.getFlags()
            flags[Detectors.SS] = True
            self.local_settings.setFlags(flags)
        else:
            flags = self.local_settings.getFlags()
            flags[Detectors.SS] = False
            self.local_settings.setFlag(flags)

    def initComponents(self):

        self.setLayout(BoxLayout(self, BoxLayout.Y_AXIS))

        self.checkbox_BOW_FLAG = JCheckBox(DetectorNames[Detectors.BOW_FLAG], actionPerformed=self.checkBoxBOW_FLAG)
        self.add(self.checkbox_BOW_FLAG)

        self.checkbox_BOW_LOGO = JCheckBox(DetectorNames[Detectors.BOW_LOGO], actionPerformed=self.checkBoxBOW_LOGO)
        self.add(self.checkbox_BOW_LOGO)

        self.checkbox_CV_FACE = JCheckBox(DetectorNames[Detectors.CV_FACE], actionPerformed=self.checkBoxCV_FACE)
        self.add(self.checkbox_CV_FACE)

        self.checkbox_WIRE = JCheckBox(DetectorNames[Detectors.WIRE], actionPerformed=self.checkBoxWIRE)
        self.add(self.checkbox_WIRE)

        self.checkbox_SS = JCheckBox(DetectorNames[Detectors.SS], actionPerformed=self.checkBoxSS)
        self.add(self.checkbox_SS)



    def customizeComponents(self):
        flags = self.local_settings.getFlags()
        self.checkbox_BOW_FLAG.setSelected(flags[Detectors.BOW_FLAG])
        self.checkbox_BOW_LOGO.setSelected(flags[Detectors.BOW_LOGO])
        self.checkbox_CV_FACE.setSelected(flags[Detectors.CV_FACE])
        self.checkbox_WIRE.setSelected(flags[Detectors.WIRE])
        self.checkbox_SS.setSelected(flags[Detectors.SS])

    def getSettings(self):
        return self.local_settings

# This is the Factory class that creates our ingest class
class CVACIngestModuleFactory(IngestModuleFactoryAdapter):

    moduleName = "EasyCV Ingest Module"

    def __init__(self):
        self.settings = None

    def getModuleDisplayName(self):
        return self.moduleName

    def getModuleDescription(self):
        return "CVAC interface to Autopsy"

    def getModuleVersionNumber(self):
        return "1.0"

    def isFileIngestModuleFactory(self):
        return True

    def createFileIngestModule(self, ingestOptions):
        res = CVACFileIngestModule(self.settings)
        return res

    def hasIngestJobSettingsPanel(self):
        return True

    def getDefaultIngestJobSettings(self):
        return CVACIngestModuleSettings()

    def getIngestJobSettingsPanel(self, settings):
        #if not isinstance(settings, CVACIngestModuleSettings):
        #   raise IllegalArgumentException("Expected CVACIngestModuleSettings")
        dolog("wire set {0}".format(settings.flags[Detectors.WIRE]))
        self.settings = settings
        panel = CVACIngestModuleSettingsPanel(self.settings)
        return panel

# File-level ingest module.  One gets created per thread.
class CVACFileIngestModule(FileIngestModule):

    def __init__(self, settings):
        self.localSettings = settings
        self.detectors = [] # this a list of tuplies (detector, modelfile)

    def log(self, level, msg):
         _logger.logp(level, self.__class__.__name__, inspect.stack()[1][3], msg)

    # Where any setup and configuration is done
    # 'context' is an instance of org.sleuthkit.autopsy.ingest.IngestJobContext.
    # See: http://sleuthkit.org/autopsy/docs/api-docs/3.1/classorg_1_1sleuthkit_1_1autopsy_1_1ingest_1_1_ingest_job_context.html
    # TODO: Add any setup code that you need here.

    def startUp(self, context):
        self.log(java.util.logging.Level.WARNING, "in startup")

        flags = self.localSettings.getFlags()

        try:
            self.log(java.util.logging.Level.WARNING, "creating detectors")
            det = None
            for i in range(len(flags)):
                if i == Detectors.BOW_FLAG and flags[i] == True:
                    det = ic.stringToProxy("BOW_Detector:default -p 10104")
                    det = cvac.DetectorPrxHelper.checkedCast(det)
                    if not det:
                        raise IngestModuleException("Invalid Detector service proxy for BOW Flags")
                    self.log(java.util.logging.Level.INFO, "Using detector " + DetectorNames[i])
                    self.detectors.append ((det,"bowUSKOCA.zip"))
                elif i == Detectors.BOW_LOGO and flags[i] == True:
                    det = ic.stringToProxy("BOW_Detector:default -p 10104")
                    det = cvac.DetectorPrxHelper.checkedCast(det)
                    if not det:
                        raise IngestModuleException("Invalid Detector service proxy for BOW Icons")
                    self.log(java.util.logging.Level.INFO, "Using detector " + DetectorNames[i])
                    self.detectors.append((det,"bowCorporateLogoModel.zip"))
                elif i == Detectors.CV_FACE and flags[i] == True:
                    det = ic.stringToProxy("OpenCVCascadeDetector:default -p 10102")
                    det = cvac.DetectorPrxHelper.checkedCast(det)
                    if not det:
                        raise IngestModuleException("Invalid Detector service proxy for CV Face")
                    self.log(java.util.logging.Level.INFO, "Using detector " + DetectorNames[i])
                    self.detectors.append((det, "OpencvFaces.zip"))
                elif i == Detectors.WIRE and flags[i] == True:
                    det = ic.stringToProxy("WireDiagram_Detector:default -p 10114")
                    det = cvac.DetectorPrxHelper.checkedCast(det)
                    if not det:
                        raise IngestModuleException("Invalid Detector service proxy for Wire Diagrams")
                    self.log(java.util.logging.Level.INFO, "Using detector " + DetectorNames[i])
                    self.detectors.append((det, "wirediagramTrainedData_opencv249.zip"))
                elif i == Detectors.SS and flags[i] == True:
                    det = ic.stringToProxy("ScreenShot_Detector:default -p 10112")
                    det = cvac.DetectorPrxHelper.checkedCast(det)
                    if not det:
                        raise IngestModuleException("Invalid Detector service proxy for Screen Shots")
                    self.log(java.util.logging.Level.INFO, "Using detector " + DetectorNames[i])
                    self.detectors.append((det, "ssTrainedData_opencv249_libSVM318.zip"))
                else:
                    self.detectors.append((None, "Invalid"))

            if det:
                self.log(java.util.logging.Level.INFO, "created detectors")
            else:
                self.log(java.util.logging.Level.ERROR, "No Detectors selected")
                raise IngestModuleException("No Detectors selected!")
        except Ice.ConnectionRefusedException:
            raise IngestModuleException("Cannot connect one of the detectors")


    # Where the analysis is done.  Each file will be passed into here.
    # The 'file' object being passed in is of type org.sleuthkit.datamodel.AbstractFile.
    # See: http://www.sleuthkit.org/sleuthkit/docs/jni-docs/classorg_1_1sleuthkit_1_1datamodel_1_1_abstract_file.html

    def process(self, file):

        # Skip non-files
        if ((file.getType() == TskData.TSK_DB_FILES_TYPE_ENUM.UNALLOC_BLOCKS) or
            (file.getType() == TskData.TSK_DB_FILES_TYPE_ENUM.UNUSED_BLOCKS) or
            (file.isFile() == False)):
                return IngestModule.ProcessResult.OK

        runset = cvac.RunSet()
        purpose = cvac.Purpose(cvac.PurposeType.UNPURPOSED, -1)
        tempfile = os.path.join(CVAC_DataDir, file.getName())
        extracted_file = File(tempfile)
        ContentUtils.writeToFile(file, extracted_file)

        #shutil.copyfile(localpath, CVAC_DataDir + "/" + localpath)
        #path, filename = os.path.split(localpath)
        dataroot = cvac.DirectoryPath()
        filePath = cvac.FilePath(dataroot,file.getName())
        substrate = cvac.ImageSubstrate(width=0, height=0, path= filePath)
        label = cvac.Label(False, "", None, cvac.Semantics())
        labelable = cvac.Labelable(0.0,label, substrate)
        seq = cvac.PurposedLabelableSeq(purpose, [labelable])
        runset.purposedLists = [seq]

        detectorRoot = cvac.DirectoryPath("detectors")

        adapter = ic.createObjectAdapter("")
        cbID = Ice.Identity()
        cbID.name = randomUUID().toString()

        cbID.category = ""
        # DetectorCallbackHanderI is a java callback.  This is required to get ICE
        # callback despatcher to find the callback since it will not find a
        # python one.
        callback = DetectorCallbackHandlerI()

        adapter.add(callback, cbID)
        adapter.activate()

        flags = self.localSettings.getFlags()
        numDet = len(flags)
        artifact = False

        for i in range(numDet):
            if flags[i] == True:
                det, model  = self.detectors[i]
                if not det:
                    continue
                if not model:
                    continue
                detectorProps = det.getDetectorProperties()
                if not detectorProps:
                    dolog("detector props returned None")
                modelPath = cvac.FilePath(detectorRoot, model)
                det.ice_getConnection().setAdapter(adapter)
                self.log(java.util.logging.Level.INFO, DetectorNames[i])
                det.process(cbID, runset, modelPath, detectorProps )
                '''
                dolog("calling begin")
                asyncRes = det.begin_process(cbID, runset, modelPath, detectorProps)
                dolog("called begin")
                asyncRes.waitForCompleted()
                dolog("wait completed")
                '''
                labelText = None
                # The java callback returns an array of resultSets.
                # It seems to work better to access these java arrays via indexes
                # instead of normal python syntax.
                resultSetArray = callback.getResults()

                for x in range(len(resultSetArray)):
                    resList = resultSetArray[x]
                    for j in range(len(resList.results)):
                        res = resList.results[j]
                        for k in range(len(res.foundLabels)):
                            labelable = res.foundLabels[k]
                            if labelable.lab.hasLabel:
                                labelText = labelable.lab.name
                                self.log(java.util.logging.Level.INFO, "Found label " + labelText)
                                # Lets just use the first label we get
                                break
                if labelText:
                    artifact = True
                    art = file.newArtifact(BlackboardArtifact.ARTIFACT_TYPE.TSK_KEYWORD_HIT)
                    att = BlackboardAttribute(BlackboardAttribute.ATTRIBUTE_TYPE.TSK_KEYWORD.getTypeID(),
                                  CVACIngestModuleFactory.moduleName, DetectorNames[i], labelText)
                    art.addAttribute(att)
                    art = file.newArtifact(BlackboardArtifact.ARTIFACT_TYPE.TSK_GEN_INFO)
                    att = BlackboardAttribute(BlackboardAttribute.ATTRIBUTE_TYPE.TSK_TAG_NAME.getTypeID(),
                                  CVACIngestModuleFactory.moduleName, DetectorNames[i], labelText)
                    art.addAttribute(att)

        if artifact:
            # Fire an event to notify the UI and others that there is a new artifact
            IngestServices.getInstance().fireModuleDataEvent(
                ModuleDataEvent(CVACIngestModuleFactory.moduleName,
                                BlackboardArtifact.ARTIFACT_TYPE.TSK_KEYWORD_HIT, None))
            IngestServices.getInstance().fireModuleDataEvent(
                ModuleDataEvent(CVACIngestModuleFactory.moduleName,
                                BlackboardArtifact.ARTIFACT_TYPE.TSK_GEN_INFO, None))


        return IngestModule.ProcessResult.OK

    def shutDown(self):

        pass
