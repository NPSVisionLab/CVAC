% This file was added by Juan
% This file is like "runDetector.m" with the difference that this works for
% multi-scales, and for the whole image, instead of only a small bounding
% box

%clear all
parameters_head;
colors = 'rgby';
scl=0.8; % This is the downscale factor, so the detector is applied to images with different scales;
MaxObjectSize = [45 45]; %= [max height, max width])
MinObjectSize = normalizedObjectSize;  % we can change it later if we want to detect smaller object
ntimes = max(MaxObjectSize./MinObjectSize); % How many times the max object is bigger than the smallest object?
Nscales=ceil(log2(ntimes)/log2(1/scl));

% Load detector parameters:
if ~exist ('data.detector'),
    load (dataFile);
    load ('data/databaseStruct');
    data.databaseStruct = D;
    NweakClassifiers = length(data.detector);
end
%NweakClassifiers = [120]; % put a list if you want to compare performances with different number of weak learners.
%NweakClassifiers = [30 120]; % put a list if you want to compare performances with different number of weak learners.
% Define variables used for the precision-recall curve
scoreAxis = linspace(0, 300, 60); 

%readerobj = mmreader('D:\MyWorks\MOV026.avi','tag', 'myreader1');
readerobj = mmreader('D:\MyWorks\segments\MOV01B_seg.avi','tag', 'myreader1');
starto=0;
%starto=21424;
%endo=21664;
%starto=1;
endo=300;
%movie(mov, 1, readerobj.FrameRate);
NtestImages=endo-starto;
% Loop on test images
ii=1;
for i = 1:1:NtestImages,
    vidFrame = read(readerobj,i+starto);  
    % Read image
    % vidFrame=imresize(vidFrame,[480 800]);
    %Img=imresize( vidFrame, 0.7, 'bilinear');
    Img=vidFrame;
    %Img=imresize( vidFrame, 0.35, 'bilinear'); 
    Img=Img(95:671,1:1024,1:3);
    %Img=Img(34:235,:,:);
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
          save (detectFile,'detection');
    toc
    
   
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % VISUALIZATION
    figure,imshow(Img);
     for j=1:Nclasses-1,
       sb = find(boxScores{j}>scoreAxis(3)); % show bounding boxes with a high score.
       plotBoundingBoxColor(boundingBox{j}(sb,:), colors(j), fix(boxScores{j}(sb)/40)+1); 
     end       
    
    str = num2str(i+1000);
    filename=strcat('figures/avi/',str,'.jpg');
    saveas(gcf,filename, 'jpg');
    close;
    ii=ii+1;
end

save (dataFile, 'data')