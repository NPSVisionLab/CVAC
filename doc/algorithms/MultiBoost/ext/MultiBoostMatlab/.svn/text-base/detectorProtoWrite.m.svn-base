% TRAINDETECTORDATAWRITE.M
% File written by Jonathan Coon on 8/3/2012 

% clear all

% load createDictionaryOriginal1.mat

% ------------------------------------------------------------------------

% clear all
% close all
% 
% % Load parameters
% parameters
% 
% % Load precomputed features for training and test
% load (dataFile)

% ------------------------------------------------------------------------

detectorProto = pb_read_trainingData__Detector();
detectorProto = pblib_set(detectorProto, 'objsizeH', data.dictionary.normalizedObjectSize(1));
detectorProto = pblib_set(detectorProto, 'objsizeW', data.dictionary.normalizedObjectSize(2));

detectorProto = pblib_set(detectorProto, 'detectEntry', pb_read_trainingData__Detector__DetectEntry());
for dl = 1:length(data.detector)
    detectorProto.detectEntry(dl) = pblib_set(detectorProto.detectEntry(dl), 'a', data.detector(dl).a);
    detectorProto.detectEntry(dl) = pblib_set(detectorProto.detectEntry(dl), 'b', data.detector(dl).b);
    detectorProto.detectEntry(dl) = pblib_set(detectorProto.detectEntry(dl), 'th', data.detector(dl).th);
    detectorProto.detectEntry(dl) = pblib_set(detectorProto.detectEntry(dl), 'bestnode', data.detector(dl).bestnode);
    detectorProto.detectEntry(dl) = pblib_set(detectorProto.detectEntry(dl), 'featureNdx', data.detector(dl).featureNdx);
    detectorProto.detectEntry(dl) = pblib_set(detectorProto.detectEntry(dl), 'b0', data.detector(dl).b0);
    detectorProto.detectEntry(dl) = pblib_set(detectorProto.detectEntry(dl), 'sharing', data.detector(dl).sharing);
    
    if dl ~= length(data.detector)
        detectorProto.detectEntry(end+1) = pb_read_trainingData__Detector__DetectEntry();
    end
    
end

% dataProto.detectorProto = detectorProto;
% dataProto.dictionaryProto = dictionaryProto;


% ------------------------------------------------------------------------

disp('buffering...')
buffer = pblib_generic_serialize_to_string(detectorProto);
fid = fopen('detector.pb', 'w');
disp('writing...');
fwrite(fid, buffer, 'uint8');
fclose(fid);
% disp('buffering...')
% buffer2 = pblib_generic_serialize_to_string(detectorProto);
% fid = fopen('trainingData6.pb', 'w');
% disp('writing...');
% fwrite(fid, buffer2, 'uint8');
% fclose(fid);
% disp('done');
% save (dataFile, 'data');