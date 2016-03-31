
import cvac

def getLabelableFilePath(lab):
    #if lab.sub.ice_isA('::cvac::ImageSubstrate'):
    if isinstance(lab.sub, cvac.ImageSubstrate):
        path = lab.sub.path
    else:
        path = lab.sub.videopath
    return path
