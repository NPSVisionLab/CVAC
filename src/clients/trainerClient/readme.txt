trainerClient
-----------------------
This is a simple command-line client that will attempt to connect to 
an Ice-enabled trainer service.

Usage:
    trainerClient.exe <detectorTrainer> <directoryTrain>
     <detectorTrainer>: The name of the detectorTrainer 
                  (found in the config.client file)
     <directoryTrain>: The directory containing images and a list file (ListTrain.txt) to train
    
-Ensure the requested service is running in an accessable IceBox instance
-Ensure the config.client is located in the current working directory.  It is best to run from the CVAC root directory


Example:
> trainerClient bowIceTrain trainImgs
