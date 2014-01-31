'''
Parses VATIC files.
matz, Dec 2013.

As per https://raw.github.com/cvondrick/vatic/master/README:
The parser expects a txt file in wich each line contains one
annotation. Each line contains 10+ columns, separated by spaces. The
definition of these columns are:

    1   Track ID. All rows with the same ID belong to the same path.
    2   xmin. The top left x-coordinate of the bounding box.
    3   ymin. The top left y-coordinate of the bounding box.
    4   xmax. The bottom right x-coordinate of the bounding box.
    5   ymax. The bottom right y-coordinate of the bounding box.
    6   frame. The frame that this annotation represents.
    7   lost. If 1, the annotation is outside of the view screen.
    8   occluded. If 1, the annotation is occluded.
    9   generated. If 1, the annotation was automatically interpolated.
    10  label. The label for this annotation, enclosed in quotation marks.
    11+ attributes. Each column after this is an attribute.

Note: Frame numbers start with '0' as the first frame.  This convention
is used by the CVAC code as well.
'''

import cvac

def parse( CVAC_DataDir, localDir,
           vidfile, framefolder, annotfile ):
    print('vatic.parse called with: {0}, {1}, {2}'
          .format( vidfile, framefolder, annotfile ))

    # what's the file name, where is it located?  both on file
    # system as well as in CVAC terms:
    cvacDir = cvac.DirectoryPath( localDir )
    cvacFp = cvac.FilePath( cvacDir, vidfile )
    substrate = cvac.Substrate( False, True, cvacFp, -1, -1 )
    fsPath = CVAC_DataDir + '/' + localDir + '/' + annotfile
    af = open( fsPath, 'r' )

    # parse line by line and assemble lines with the same
    # trackID into a CVAC LabeledTrack.  Some checks are performed
    # to see whether the output format is what we expect.
    labels = []
    paths = {}
    lastTrackID = -1
    warned_lost = False
    lbltrack = None
    for line in af:
        cols = line.strip().split(' ')  # line has \n - remove it
        trackID = int(cols[0])
        xmin    = int(cols[1])
        ymin    = int(cols[2])
        xmax    = int(cols[3])
        ymax    = int(cols[4])
        frame   = int(cols[5])
        lost    = cols[6]
        occluded   = cols[7]
        generated  = cols[8]
        labelname  = cols[9].strip('"')  # single label, enclosed in ""
        attributes = cols[10:]          # attribute columns, if any
        # print trackID, xmin, ymin, labelname, attributes

        # ignore lost and occluded labels
        if lost=="1" or occluded=="1":
            if not warned_lost:
                print("warning: lost or occluded track, will be included in labels")
                warned_lost = True

        # format consistency checks
        if trackID==-1 or trackID<lastTrackID or trackID>lastTrackID+1:
            raise RuntimeError("unexpected trackID: {0} (was {1})."
                               .format(trackID, lastTrackID))

        # new track or continuation of previous track?
        if trackID==lastTrackID:
            # make sure labels haven't changed
            if lbltrack.lab.name!=labelname:
                raise RuntimeError(
                    "in trackID {0}, expected identical labels ({0} vs {1})"
                    .format( lbltrack.lab.name, labelname ))
        else:
            lbltrack = cvac.LabeledTrack()
            labels = labels + [lbltrack]
            lastTrackID = trackID

            lbltrack.confidence = 1.0
            properties = {}
            for attrib in attributes:
                properties[ attrib.text ] = ''
            lbltrack.lab = cvac.Label( True, labelname, properties, cvac.Semantics())
            lbltrack.sub = substrate
            lbltrack.keyframesLocations = []
            lbltrack.interp = cvac.Interpolation.DISCRETE

        vst = cvac.VideoSeekTime( -1, frame )  # no time, but frame number
        loc = cvac.BBox(xmin, ymin, xmax-xmin, ymax-ymin)
        frameloc = cvac.FrameLocation( vst, loc )
        lbltrack.keyframesLocations.append( frameloc )

    return labels

