% This file was added by Juan
% This file is like "runDetector.m" with the difference that this works for
% multi-scales, and for the whole image, instead of only a small bounding
% box

clear all

parameters
testImageSize = [256 256];
resampl=0.5; % We will downscale the original image so we get a faster result.
scl=0.8; % This is the downscale factor, so the detector is applied to images with different scales;
MaxObjectSize = [491 181]; %= [max height, max width])
MinObjectSize = normalizedObjectSize;  % we can change it later if we want to detect smaller object
ntimes = max(MaxObjectSize./MinObjectSize); % How many times the max object is bigger than the smallest object?
Nscales=ceil(log2(ntimes)/log2(1/scl));

% Load detector parameters:
load (dataFile);
load ('data/databaseStruct');
%load ('data/databaseSets');
NweakClassifiers = length(data.detector);
%NweakClassifiers = [120]; % put a list if you want to compare performances with different number of weak learners.

% Define variables used for the precision-recall curve
scoreAxis = linspace(0, 300, 60); RET = []; REL = []; RETREL = [];

% Create figures
ii=1;
% Loop on test images
for obj = 1:(Nclasses-1),    
    [Dc, jc]  = LMquery(data.databaseStruct, 'object.name', objects.name{obj});
    trainedImages=data.image(find(data.class==obj));
    % remove images used to create the dictionary:
    testImages = setdiff(jc, [trainedImages; data.dictionary.imagendx']);
    NtestImages = length(testImages);

    % Normalize image:
    %[newannotation, newimg, crop, scaling, err, msg] = LMcookimage(annotation, Img, ...
    %    'objectname', objects, 'objectsize', normalizedObjectSize, 'objectlocation', 'original', 'maximagesize', testImageSize);
    
    % Remove black borders of the image on the Y axis (if they exist) and
    % update the annotation.
     % Loop on test images
    for i = 1:NtestImages
        % Read image and ground truth
        Img = LMimread(data.databaseStruct, testImages(i), HOMEIMAGES);
        annotation = data.databaseStruct(testImages(i)).annotation;
        
        [Img,annotation]=removeBorders(Img,annotation);
        img=Img; 
        [rows cols dim]=size(img);
        for j=1:Nclasses-1,
            boundingBox{j}=[];
            boxScores{j}=[];
        end
        Score=zeros(rows,cols,Nclasses-1,Nscales);
        tic
            for scale = 1:Nscales,
            % Run derector at multiple scales (you can loop on scales to get
            % scale invariance):
                [Scorei, boundingBoxi, boxScoresi] = singleScaleJointDetector(double(mean(img,3)), data, T , NweakClassifiers(1));
                for j=1:Nclasses-1,
                    boundingBox{j}=[boundingBox{j};boundingBoxi{j}.*(scl^-(scale-1))]; % Rescale the bounding box
                    Score(:,:,j,scale) = imresize(Scorei(j), [rows cols], 'bilinear');
                    boxScores{j}=[boxScores{j};boxScoresi{j}];
                    detection{ii}.boundingBox{j} = boundingBox{j}; 
                    detection{ii}.boxScores{j} = boxScores{j}; 
                end
                % Re-scale the image
                img = imresize(img, scl, 'bilinear');      
            end
        toc
        ii=ii+1;
    end
end

%Added by Juan
data.RET = RET;
data.REL = REL;
data.RETREL=RETREL;
save (dataFile, 'data')
save (detectFile,'detection');