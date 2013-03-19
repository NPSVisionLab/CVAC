clear all
close all

% Load parameters
parameters

load (dataFile)
dictionary = data.dictionary;
D = data.databaseStruct;

Nfeatures = length(dictionary.filter);
Nsamples = (2*numTrainImages) * (negativeSamplesPerImage+1); % this is an upper bound to the number of samples that will be computed

ns = 0; % counter for the number of samples really computed.

for obt=1:Nclasses-1,
    % Query for images that contain object class objects{c}:
    [Dc, jc]  = LMquery(D, 'object.name', objects.name{obt});  
    % Remove images used to create the dictionary:
    jc = setdiff(jc, dictionary.imagendx);
    % Maker sure that the images to create the dictionary are random and not
    % sequencies
    
    rind=randperm(length(jc));
    jc=jc(rind);

    i = 0; 
    ii = 0;
    while ii < numTrainImages & i < length(jc)
        i = i+1;
        img = LMimread(D, jc(i), HOMEIMAGES);
        img = uint8(mean(double(img),3));

        annotation = D(jc(i)).annotation;
        jo = LMobjectindex(annotation, objects.name{obt});  %specific instances in the image of class type
        % Loop on the number of instances present in each image
        imageUsed = 0;
        for m = 1:length(jo)  
            ann = annotation;
            ann.object = ann.object([jo(m) setdiff(jo,jo(m))]); % the object of the specific class
            
            
            % Get tight crop of the centered object to extract patches:
            [newannotation, newimg, crop, scaling, err, msg] = LMcookimage(ann, img, ...
                'objectname', objects.name{obt}, 'objectsize', normalizedObjectSize, 'objectlocation', 'original', 'maximagesize', trainingImageSize);
                     
            [nrows, ncols] = size(newimg);

            if err == 0
                % Get object polygon
                [X,Y] = LMobjectpolygon(newannotation, objects.name{obt}); % get object polygon
                if min(X{1})>-1 & min(Y{1})>-1 & max(X{1})<ncols+1 & max(Y{1})<nrows+1;    imageUsed = 1; end
            else
                disp(msg)
            end
        end
        ii = ii + imageUsed;;
    end
    ii
end
