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
import cvac
import xml.etree.ElementTree as et
import glob
import os

def parsePolygon( etelem ):
    polygon = cvac.Silhouette()
    polygon.points = []
    for pt in etelem.findall('pt'):
        polygon.points = polygon.points \
          + [cvac.Point2D(pt.find('x').text, pt.find('y').text)]
    return polygon

def parseLabeledObjects( root, substrate ):
    ''' for each labeled object in the annotation tree
    that does not have the <deleted> tag set,
    collect the name, attributes and the polygon and create the
    equivalent cvac.Silhouette from it
    '''
    labels = []
    for lmobj in root.findall('object'):
        deleted = lmobj.find('deleted').text
        if deleted=='1':
            continue
        label = cvac.LabeledLocation()
        label.confidence = 1.0
        label.sub = substrate
        name = lmobj.find('name').text
    
        properties = {}
        for attrib in lmobj.findall('attributes'):
            if not attrib.text: break
            properties[ attrib.text ] = ''
        label.lab = cvac.Label( True, name, properties, cvac.Semantics() )
        label.loc = parsePolygon( lmobj.find('polygon') )

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
    fsAnnotPath = os.path.join(CVAC_DataDir, localDir, lmAnnotations, lmFolder) + '/*.xml'
    for fsAnnotFullpath in glob.glob( fsAnnotPath ):
        # parse the XML file on the file system
        tree = et.parse( fsAnnotFullpath )
        root = tree.getroot()
        # find out image name, prepend image path
        cvacDir = cvac.DirectoryPath( os.path.join(localDir, lmImages, lmFolder ))
        imgFname = root.find('filename').text
        cvacFp = cvac.FilePath( cvacDir, imgFname )
        substrate = cvac.Substrate( True, False, cvacFp, -1, -1 )
        labels = labels + parseLabeledObjects( root, substrate )
    return labels
