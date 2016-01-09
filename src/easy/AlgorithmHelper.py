'''
Created on Jan 6, 2016

@author: tomb
'''

from collections import namedtuple

class AlgorithmHelper:
    '''
    This class Helps select the appropriate trainer give the attributes of
    the size of the training set, the image types, and the speed of the detection algorithm.
    '''

    class TrainerDataSize: (Small, Medium, Large, VeryLarge) = range(4) 
    class TrainerImageType: (Simple, Complex, MultiObject) = range(3)
    class DetectionSpeed: (Fast, Medium, Slow, VerySlow) = range(4)

    #TrainerDescription = namedtuple('TrainerDescription', 'data, imageType, speed')
    TrainerDescription = namedtuple('TrainerDescription', 'data')
    def __init__(self):
        '''
        Constructor.  Build the attribute dictionary of the different trainers.
        '''
        self.trainers = {}
        '''
        self.trainers["BOW_Trainer"] =  self.TrainerDescription(self.TrainerDataSize.Small, 
                                                        self.TrainerImageType.MultiObject,
                                                        self.DetectionSpeed.Medium       
                                                                   )
        self.trainers["OpenCVCascadeTrainer"] = self.TrainerDescription(self.TrainerDataSize.Large,
                                                                 self.TrainerImageType.Simple,
                                                                 self.DetectionSpeed.Fast
                                                                 )
        '''
        self.trainers["BOW_Trainer"] =  self.TrainerDescription(self.TrainerDataSize.Small
                                                                   )
        self.trainers["OpenCVCascadeTrainer"] = self.TrainerDescription(self.TrainerDataSize.Large
                                                                 )
        
    def recommendTrainer(self, dataSize = None, imageType = None, detectionSpeed = None):
        for k, v in self.trainers.iteritems():
            if v.data == dataSize or not dataSize:
                #if v.imageType == imageType or not imageType:
                #   if v.speed == detectionSpeed or not detectionSpeed:
                return k
        return None
    
    def getTrainerDescription(self, trainer):
        for k, v in self.trainers.iteritems():
            if k == trainer:
                return v
            
    def listTrainers(self):
        res = []
        for k, v in self.trainers.iteritems():
            res.append(k)
        return res
    
'''
Test
'''
if __name__ == '__main__':
    ah = AlgorithmHelper()
    trainers =  ah.listTrainers()
    for t in trainers:
        td = ah.getTrainerDescription(t)
        print("Trainer " + t)
        print("trainer data {0}".format(td.data))
        #print("trainer imageType {0}".format(td.imageType))
        #print("trainer speed {0}".format(td.speed))
        #rec = ah.recommendTrainer(td.data, td.imageType, td.speed)
        rec = ah.recommendTrainer(td.data)
        print("Recommended trainer " + rec)
    
    
                