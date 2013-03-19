clear all
close all

% Load parameters
parameters;

% Load precomputed features for training and test
load (dataFile);
load ('data/databaseSets');
load ('data/databaseStruct');
classifier=data.detector;
fN=[];
vecnodes=[];
for i=1:length(classifier),
    vecnodes=[vecnodes classifier(i).bestnode]; % All the nodes in the rounds
    fN=[fN classifier(i).featureNdx]; % All the feature numbers used in the rounds (each round one).
end

hist(vecnodes,max(vecnodes)*2);  
ylabel('Frequency'); xlabel('Node No.');

Ndic=length(data.dictionary.imagendx);
Cdic=Ndic/(Nclasses-1);
fC=fix(fN/Cdic)+1;     % The classes that each feature belong

figure;
hist(fC,max(fC));  
ylabel('Frequency'); xlabel('Class No.');
h = findobj(gca,'Type','patch');
set(h,'FaceColor','r','EdgeColor','w')


im_used=data.dictionary.imagendx(fN);

% Present the popularity of the images used to create the dictionary (ans show the last popluar, per class). 
for cl=1:(Nclasses-1),
    figure;
    iDx=im_used(find(fC==cl));
    hist(iDx,unique(iDx));
    [N,X]=hist(iDx,unique(iDx));
    [val ind]=min(N);
    lowpic=X(ind);
    img = LMimread(D, lowpic, HOMEIMAGES);
    img = uint8(mean(double(img),3));  %Convert to grayscale 
    [newannotation, newimg, crop, scaling, err, msg] = LMcookimage(D(lowpic).annotation, img, ...
    'objectname', objects.name{cl}, 'objectsize', normalizedObjectSize, 'objectlocation', 'centered', 'maximagesize', dictionaryImageSize);
    figure,imshow(newimg);
end