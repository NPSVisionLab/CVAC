'''A very basic client for LabelMe servers,
for now just for local file system access to the annotations
and image references.

Usage:
Collect and parse all XML files in this directory:
lmAnnotations = '/some/folder/to/LabelMe/Annotations'
lmFolder = 'example_folder'
labels = labelme.parseFolder( lmAnnotations, lmFolder )
print 'found a total of ' + str(len(labels)) + ' labels'
'''
from __future__ import print_function
from future import standard_library
standard_library.install_aliases()
import cvac
import xml.etree.ElementTree as et
import glob
import os
import urllib.request, urllib.parse, urllib.error

def parsePolygon( etelem ):
    polygon = cvac.Silhouette()
    polygon.points = []
    for pt in etelem.findall('pt'):
        num = pt.find('x').text.encode('utf-8').strip()
        if '.' in num: 
            tx = int(float(num))
        else:
            tx = int(num)
        num = pt.find('y').text.encode('utf-8').strip()
        if '.' in num:  
            ty = int(float(num))
        else:
            ty = int(num)
        # + [cvac.Point2D(str(tx-1), str(ty-1))]
        polygon.points = polygon.points + [cvac.Point2D((tx-1), (ty-1))]
    return polygon

def parseLabeledObjects( root, imageSubstrate ):
    ''' for each labeled object in the annotation tree
    that does not have the <deleted> tag set,
    collect the name, attributes and the polygon and create the
    equivalent cvac.Silhouette from it
    '''    
    objImgSize = root.find('imagesize')
    if objImgSize != None:
        objH = objImgSize.find('nrows')
        if objH != None and objH.text != None:            
            imageSubstrate.height = int(objH.text.encode('utf-8').strip())
            
        objW = objImgSize.find('ncols')
        if objW != None and objW.text != None:
            imageSubstrate.width = int(objW.text.encode('utf-8').strip())
    
    labels = []
    for lmobj in root.findall('object'):
        deleted = lmobj.find('deleted').text.encode('utf-8').strip()
        if deleted=='1':
            continue
        label = cvac.LabeledLocation()
        label.confidence = 1.0
        label.sub = imageSubstrate
        nameobj = lmobj.find('name')
        if nameobj.text is None:
            continue
        #name = nameobj.text.strip()
        name = nameobj.text.encode('utf-8').strip()
    
        properties = {}
        for attrib in lmobj.findall('attributes'):
            if not attrib.text: break
            properties[ attrib.text.encode('utf-8').strip() ] = ''
        label.lab = cvac.Label( True, name, properties, cvac.Semantics() )
        label.loc = parsePolygon( lmobj.find('polygon') )
        
        #assumption: 0-based notation
        for pt in label.loc.points:
            if (int(pt.x)<0) or (int(pt.y)<0):
                print("Warning: label \"" \
                      + name + "\" is out of bounds in file \"" \
                      + label.sub.path.filename + "\"")

        labels = labels + [label]

    # If we don't have any object attributes maybe we have a imageattribute and
    # we can get the label from there.
    if labels == []:
        labelName = ''
        for aobj in root.findall('imageattribute'):
            properties = {}
            walkAll = aobj.getchildren()
            pname = ''
            pvalue = ''
            for elt in walkAll:
                if labelName == '' and elt.tag == 'name':
                    labelName = elt.text.encode('utf-8').strip()
                else:
                    # If we have a value create a property entry with key as the name
                    # If we only have a name and no value then
                    # ignore the name if not the first.
                    if elt.tag == 'name':
                        pname = elt.text.encode('utf-8').strip()
                    elif elt.tag == 'value':
                        pvalue = elt.text.encode('utf-8').strip()
                    if pname != '' and pvalue != '':
                        properties[pname] = pvalue
                        pname = ''
                        pvalue = ''
            
        if labelName != '':
           # we got a name from imageattributes
            label = cvac.LabeledLocation()
            label.confidence = 1.0
            label.sub = cvac.Substrate()             
            label.lab = cvac.Label( True, labelName, properties, cvac.Semantics() )  
            labels = labels + [label]   
            
    return labels

def parseFolder( localDir, lmAnnotations, lmImages, lmFolder, CVAC_DataDir ):
    '''Parse all XML files in the specified folder and
    return all found labels.
    (this currently only works locally on the file system)
    lmAnnotations is equivalent to HOMEANNOTATIONS in the Matlab LabelMeToolbox:
    it is the path to the Annotation root folder on the file system, 
    or the LabelMe server's Annotation folder http address.
    lmFolder is the equivalent to HOMEIMAGESin the Matlab LabelMeToolbox.
    Both lmAnnotations and lmFolder are assumed to be in localDir
    '''

    labels = []
    fsAnnotPath = os.path.join(CVAC_DataDir, localDir, lmAnnotations, lmFolder) \
                  + '/*.xml'
                  
    for fsAnnotFullpath in glob.glob( fsAnnotPath ):
        # parse the XML file on the file system
        tree = et.parse( fsAnnotFullpath )
        root = tree.getroot()
        # find out image name, prepend image path
        cvacDir = cvac.DirectoryPath( os.path.join(localDir, lmImages, lmFolder ))
        felem = root.find('filename')
        if felem == None:
            print('Annotation file ' + fsAnnotFullpath + \
                  ' does not have filename element')
            continue
        else:
            imgFname = felem.text.encode('utf-8').strip() # strip any leading or trailing white space
            
        cvacFp = cvac.FilePath( cvacDir, imgFname )
        substrate = cvac.ImageSubstrate( -1, -1, cvacFp)
        labels = labels + parseLabeledObjects( root, substrate )
    return labels

def downloadImages( lmImages, lmFolder, CVAC_DataDir, toLocalDir ):
    # note: remote directories need to have the trailing slash /
    remoteDirName = lmImages + "/Images/" + lmFolder + "/"
    localDirFilename = toLocalDir + "/dirlist.html"
    # print("debug: fetching directory listing from " + remoteDirName)
    if not os.path.isdir( toLocalDir ):
        os.makedirs( toLocalDir )
    urlfile = urllib.request.URLopener()
    localfname = urlfile.retrieve( remoteDirName, localDirFilename )

    # parse for images: look for a link to a jpg with the same name
    # <a href="img_0868.jpg">img_0868.jpg</a>
