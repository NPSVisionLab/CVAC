'''now just for local file system access to the annotations
and video references.

Usage:
Parse XML catalog files, 
then all xml files indicated by catalog files are parsed, 
then video information including segments will be collected
'''
import cvac
import xml.etree.ElementTree as et
import os
from xml.dom import minidom

class TRECVIDAnnotaitonParser():
    ''' for each annotation file, 
    collect frame information of both hart cuts and soft cuts    
    '''
    def __init__(self, xmlpath):        
        self.xmlpath = xmlpath; 
        self.cutSoft = [];
        self.cutHard = [];  
        self.cutAll = [];     
        if os.path.exists(self.xmlpath):
            xmldoc = minidom.parse(self.xmlpath)
            itemlist = xmldoc.getElementsByTagName('trans')            
            nSoft = 0
            nHard = 0
            nAll = 0
            for s in itemlist:
                tlist = [s.attributes['preFNum'].value,
                         s.attributes['postFNum'].value,
                         s.attributes['type'].value]
                self.cutAll.append(tlist)
                nAll = nAll + 1
                if s.attributes['type'].value.lower()=='cut':
                    self.cutHard.append(tlist)
                    nHard = nHard + 1
                else:
                    self.cutSoft.append(tlist)
                    nSoft = nSoft + 1
            #===================================================================
            # print(str(nAll)+"(Hard="+str(nHard)+"/Soft="+str(nSoft)+")" \
            #       + " elements are obtained from " + self.xmlpath)
            #===================================================================
        else:            
            raise RuntimeError("No xml file")
        
    def getCutSoft(self):
        return self.cutSoft
    
    def getCutHard(self):
        return self.cutHard
    
    def getCutAll(self):
        return self.cutAll
    
    
def parseSamples( root, CVAC_DataDir ):
    ''' for each catalog file, 
    collect all samples including videos files and their annotation files
    then, each segment information is converted to cvac.LabeledVideoSegment    
    '''
    dirCommon = ''
    dir = root.find('directory')
    if dir != None:
        dirCommon = dir.text + '/'    
    
    labels = []
    for sample in root.findall('sample'):
        media = sample.find('media')
        mediaLocal = media.find('local_media')
        mediaDir = dirCommon        
        if mediaLocal.find('relativePath') != None:
            mediaDir = mediaDir + mediaLocal.find('relativePath').text + '/'
        mediaName = mediaLocal.find('filename').text
        mediaFilepath = cvac.FilePath(cvac.DirectoryPath(mediaDir),mediaName)
        
        annot = sample.find('annotation')
        annotLocal = annot.find('local_annotation')
        annotDir = dirCommon        
        if annotLocal.find('relativePath') != None:
            annotDir = annotDir + annotLocal.find('relativePath').text + '/'
        annotName = annotLocal.find('filename').text
        
        annotObj = TRECVIDAnnotaitonParser(CVAC_DataDir+'/'+annotDir+annotName)
        #print(CVAC_DataDir+'/'+annotDir+annotName)
        bndFrms = annotObj.getCutAll() 
        #annotObj.getCutHard() for HardCuts
        #annotObj.getCutSoft() for SoftCuts        
        for frm in bndFrms:
            label = cvac.LabeledVideoSegment()
            label.confidence = 1.0        
            label.lab = cvac.Label(True,(sample.find('nickname')).get('uniqueid'))        
            label.sub = cvac.Substrate(False,True,mediaFilepath,-1,-1)
            label.start = cvac.VideoSeekTime(frm[0],-1)
            label.last = cvac.VideoSeekTime(frm[1],-1)
            label.startAfterTx = cvac.VideoSeekTime(-1,-1)
            label.lastBeforeTx = cvac.VideoSeekTime(-1,-1)
            label.loc = cvac.Location()
            labels = labels + [label]        
            
    return labels

def parseCatalog( CVAC_DataDir, localDir, catalogFile ):
    '''Parse XML catalog files, and then 
    return all found labels.
    '''

    labels = []
    catalogPath = os.path.join(CVAC_DataDir, localDir, catalogFile) + '.xml'
    
    tree = et.parse( catalogPath )
    root = tree.getroot()
    
    labels = labels + parseSamples( root, CVAC_DataDir )
    
    return labels