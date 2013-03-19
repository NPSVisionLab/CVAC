% TRAININGDATAREAD.M 
% File created by Jonathan Coon on 8/7/2012 
% ------------------------------------------------------------------------

% clear all
% close all
% 
% % Load parameters
% parameters
% 
% % Load precomputed features for training and test
% load (dataFile)

disp('reading...');
fid = fopen('detector.pb');
buffer = fread(fid, [1 Inf], '*uint8');
fclose(fid);
data.detectorProto = pb_read_trainingData__Detector(buffer);
disp('done');
% save (dataFile, 'data');