clear all
parameters
load (dataFile);
load (detectFile);
load ('data/databaseStruct');
data.databaseStruct = D;
scoreAxis = linspace(0, 300, 60);
k=0.8 % scaling factor
min_neighbors=1; % minimum neighbohors. 
colors = 'rgby';

ii=1;
for obj = 1:(Nclasses-1),    
    [Dc, jc]  = LMquery(data.databaseStruct, 'object.name', objects.name{obj});
    trainedImages=data.image(find(data.class==obj));
    % remove images used to create the dictionary:
    testImages = setdiff(jc, [trainedImages; data.dictionary.imagendx']);
    NtestImages = length(testImages);
    for i = 1:1,
        figure;
        %for i = 1:NtestImages
        % Read image and ground truth
        Img = LMimread(data.databaseStruct, testImages(i), HOMEIMAGES);
        annotation = data.databaseStruct(testImages(i)).annotation;

        % Normalize image:
        %[newannotation, newimg, crop, scaling, err, msg] = LMcookimage(annotation, Img, ...
        %    'objectname', objects, 'objectsize', normalizedObjectSize, 'objectlocation', 'original', 'maximagesize', testImageSize);

        % Remove black borders of the image on the Y axis (if they exist) and
        % update the annotation.
        [Img,annotation]=removeBorders(Img,annotation);
        
        for j=1:Nclasses-1,
            boundingBox{j}= detection{ii}.boundingBox{j}; 
            boxScores{j}=detection{ii}.boxScores{j}; 
        end
        
       
        imshow(Img);
        
        boundingBoxto=[];
        boxScoresto=[];
        clases=[];
        
        for j=1:Nclasses-1,
            sb = find(boxScores{j}>scoreAxis(20)); % show bounding boxes with a high score.
            plotBoundingBoxColor(boundingBox{j}(sb,:), colors(j), fix(boxScores{j}(sb)/40)+1); 
            % get the boxes regardless the class, the image, upper certain
            % a high score.
            boundingBoxto=[boundingBoxto;boundingBox{j}(sb,:)];
            boxScoresto=[boxScoresto;boxScores{j}(sb)];
            clases=[clases; (ones(length(sb),1)*j)];
        end
        figure;
        imshow(Img);
       %[boxes,scores] = GatherDetections(boundingBox(sb,:),boxScores(sb),k,min_neighbors); % HERE IS THE GROUPING OF THE RESULTS
       [boxes,scores,clases] = GatherDetections(boundingBoxto,boxScoresto,clases,k,min_neighbors); % HERE IS THE GROUPING OF THE RESULTS
       
       for k=1:size(boxes,1)
         plotBoundingBoxColor(boxes(k,:), colors(clases(k)) , fix(scores(k,:)/40)+1);    
       end
       ii=ii+1;
    end  
end
