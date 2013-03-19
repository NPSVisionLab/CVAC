% clear all
% close all
% 
% % Load parameters
% parameters
% 
% % Load precomputed features for training and test
% load (dataFile)

% ------------------------------------------------------------------------

dictionaryProto = pb_read_trainingData__Data();
dictionaryProto = pblib_set(dictionaryProto, 'objsizeH', data.dictionary.normalizedObjectSize(1));
dictionaryProto = pblib_set(dictionaryProto, 'objsizeW', data.dictionary.normalizedObjectSize(2));

dictionaryProto = pblib_set(dictionaryProto, 'avgsize', pb_read_trainingData__Data__AvgSize());
for cl = 1:Nclasses-1
    dictionaryProto.avgsize(cl) = pblib_set(dictionaryProto.avgsize(cl), 'avgsizeH', abs(data.averageBoundingBox{cl}(4)-data.averageBoundingBox{cl}(3)));
    dictionaryProto.avgsize(cl) = pblib_set(dictionaryProto.avgsize(cl), 'avgsizeW', abs(data.averageBoundingBox{cl}(2)-data.averageBoundingBox{cl}(1)));
    
    if cl ~= Nclasses-1
        dictionaryProto.avgsize(end+1) = pb_read_trainingData__Data__AvgSize();
    end
    
end

dictionaryProto = pblib_set(dictionaryProto, 'filters', pb_read_trainingData__Data__Filter());
for numFilters = 1:Nfilters
    dictionaryProto.filters(numFilters) = pblib_set(dictionaryProto.filters(numFilters), 'filterData', data.dictionary.filterLin{numFilters});
    
    if numFilters ~= Nfilters
        dictionaryProto.filters(end+1) = pb_read_trainingData__Data__Filter();
    end
    
end

dictionaryProto = pblib_set(dictionaryProto, 'dictEntry', pb_read_trainingData__Data__DictEntry());
for j = 1:length(data.dictionary.filter)
    
    for numFilter = 1:Nfilters
        if data.dictionary.filterLin{j} == dictionaryProto.filters(numFilter).filterData
            filterIndex = numFilter;
        end
    end
    
    fprintf('creating dictionary entry %d\n', j);
    
    dictionaryProto.dictEntry(j) = pblib_set(dictionaryProto.dictEntry(j),'filterIndex', filterIndex);
    dictionaryProto.dictEntry(j) = pblib_set(dictionaryProto.dictEntry(j), 'patch', data.dictionary.patchLin{j});
    dictionaryProto.dictEntry(j) = pblib_set(dictionaryProto.dictEntry(j), 'location', pb_read_trainingData__Data__Location());
    dictionaryProto.dictEntry(j).location = pblib_set(dictionaryProto.dictEntry(j).location, 'x', data.dictionary.location{j}{1});
    dictionaryProto.dictEntry(j).location = pblib_set(dictionaryProto.dictEntry(j).location, 'y', data.dictionary.location{j}{2});
    dictionaryProto.dictEntry(j) = pblib_set(dictionaryProto.dictEntry(j), 'patchRo', size(data.dictionary.patch{j},1));
    dictionaryProto.dictEntry(j) = pblib_set(dictionaryProto.dictEntry(j), 'patchCo', size(data.dictionary.patch{j},2));
    dictionaryProto.dictEntry(j) = pblib_set(dictionaryProto.dictEntry(j), 'imagendx', data.dictionary.imagendx(j));

    if j ~= length(data.dictionary.filter)
        dictionaryProto.dictEntry(end+1) = pb_read_trainingData__Data__DictEntry();
    end
    
end

% ------------------------------------------------------------------------

disp('buffering...')
buffer = pblib_generic_serialize_to_string(dictionaryProto);
fid = fopen('dictionary.pb', 'w');
disp('writing...');
fwrite(fid, buffer, 'uint8');
fclose(fid);
disp('done');
% save (dataFile, 'data');