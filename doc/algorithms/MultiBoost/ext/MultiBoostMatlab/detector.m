% function detector_filenames(input,output)
% This file was added by Juan
% This file was modified by Jonathan Coon on 6/26/2012
% Last update: 7/17/2012
% This file is like "segmentVideo.m" with the difference that this works for
% multi-scales, and for the whole image, instead of only a small bounding
% box

%class 1 -- 0 degrees
%class 2 -- 90 degrees
%class 3 -- 270 degrees
%class 4 -- 180 degrees

% Media Analyst standard output testing
disp('starting detector_filenames...disp');
fprintf('%s\n','starting detector_filenames...fprintf');

% clear all
parameters
colors = 'rgby';
scl=0.8; % This is the downscale factor, so the detector is applied to images with different scales;
MaxObjectSize = [242 103]; %= [max height, max width])
% MaxObjectSize = normalizedObjectSize;
MinObjectSize = normalizedObjectSize;  % we can change it later if we want to detect smaller object
ntimes = max(MaxObjectSize./MinObjectSize); % How many times the max object is bigger than the smallest object?
Nscales=ceil(log2(ntimes)/log2(1/scl));
% Nscales=3;

% Load detector parameters:
load (dataFile);
disp('loaded dataFile');
load ('data/databaseStruct');
disp('loaded data/databaseStruct');
data.databaseStruct = D;
NweakClassifiers = length(data.detector);
% Define variables used for the precision-recall curve
scoreAxis = linspace(0, 300, 60); 

fileID = fopen('imgDetectionsProto3.txt','wt');
filenames = textread('RunSet_4.dat','%s','whitespace','\r');
% numDetect = zeros(1,length(filenames));
% numFrames = zeros(1,length(filenames));
% frameRate = zeros(1,length(filenames));

% description for text output
fprintf(fileID,'# MultiBoost Matlab results version 1.0\n');
fprintf(fileID,'# number of detections\n');
fprintf(fileID,'# bounding box coordinates are 1-based, origin top-left\n');
fprintf(fileID,'# confidence score > 0\n');
fprintf(fileID,'# torso orientation: ');
fprintf(fileID,'0: shoulder facing camera, ');
fprintf(fileID,'90: shoulder facing left of camera, ');
fprintf(fileID,'180: shoulder facing away from camera, ');
fprintf(fileID,'270: shoulder facing right of camera\n');
fprintf(fileID,'# x y w h confidence deg-azimuth\n');

% in case of reading frames from a video:
% readerobj = mmreader('D:\MyWorks\segments\MOV007_seg1.00.avi','tag', 'myreader1');
% starto=0;
% %starto=21424;
% %endo=21664;
% %starto=1;
% endo=169;
% %movie(mov, 1, readerobj.FrameRate);
% NtestImages=endo-starto;
NtestImages = size(filenames);
% Loop on test images
ii=1;
for i = 1:1:NtestImages,
%     vidFrame = read(readerobj,i+starto);  
%     % Read image
%      Img=vidFrame;
%     Img=Img(95:671,1:1024); % Take out the black stripes
%     img=Img; 
    % img = load from file with next entry from filenames array
    readImg = imread(filenames{i});
    Img=readImg;
    Img=Img(95:671,1:1024); % Take out the black stripes
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
%                      boundingBox{j} = [((boundingBox{j}(1) + boundingBox{j}(2))/2),... 
%                          ((boundingBox{j}(3) + boundingBox{j}(4))/2),...
%                          (boundingBox{j}(2) - boundingBox{j}(1)),...
%                          (boundingBox{j}(4) - boundingBox{j}(3))];
                         
                end
%                 boundingBox{j} = [350 450 125 275];  % for testing
%                 boxScores{j} = 200;
                
                % Re-scale the image
                img = imresize(img, scl, 'bilinear');      
          end
    toc
%%  
% -----------------------------------------------------------------------------
    sumbBoxes = 0;
    for j=1:Nclasses-1,
        sumbBoxes = sumbBoxes + size(boundingBox{j}, 1); % number of detections for each image
    end
    fprintf(fileID,'%d\n',sumbBoxes); % prints the number of detections for each image to text file
% -----------------------------------------------------------------------------
    [sortedBoxScores, ind] = sort(cell2mat(boxScores(:)),'descend');
    totalBoundingBox = cat(1,boundingBox{:});
    angles = zeros(1,sumbBoxes);
    sizebBoxes = zeros(1,Nclasses-1);
    cnt = 0;
    for j=1:Nclasses-1,
        sizebBoxes(j) = size(boundingBox{j}, 1); % size of boundingBox
        for k = 1:sizebBoxes(j)
		    cnt = cnt+1;
            if j == 1
                angles(cnt) = 0;
            elseif j == 2
                angles(cnt) = 90;
            elseif j == 3
                angles(cnt) = 270;
            elseif j == 4
                angles(cnt) = 180;
            end
        end
    end
% -----------------------------------------------------------------------------    
    for boxIdx = 1:sumbBoxes
        fprintf(fileID,'%d %d %d %d %3.1f %d\n',...                                              % prints each x,y,width, and heigth, confidence,
            round(totalBoundingBox(ind(boxIdx),:)),sortedBoxScores(boxIdx),angles(ind(boxIdx))); % and angle for each detection in order of highest confidence
    end
%     for boxIdx = 1:sumbBoxes
%         fprintf(fileID,'%3.1f %3.1f %3.1f %3.1f %3.1f %d\n',...                                    % prints x and y coordinate of the
%             ((totalBoundingBox(ind(boxIdx),2)) + (totalBoundingBox(ind(boxIdx),1)))/2,...          % center of each detection, and the
%                 ((totalBoundingBox(ind(boxIdx),4)) + (totalBoundingBox(ind(boxIdx),3)))/2,...      % width and height, confidence, and
%                     (totalBoundingBox(ind(boxIdx),2)) - (totalBoundingBox(ind(boxIdx),1)),...      % angle for each detection in order
%                         (totalBoundingBox(ind(boxIdx),4)) - (totalBoundingBox(ind(boxIdx),3)),...  % of highest confidence
%                             sortedBoxScores(boxIdx),angles(ind(boxIdx)));
%     end

%     for boxIdx = 1:sumbBoxes
%         fprintf(fileID,'%d %d %d %d\n',...                                    % prints x and y coordinate of the
%             round(((totalBoundingBox(ind(boxIdx),2)) + (totalBoundingBox(ind(boxIdx),1)))/2),...          % center of each detection, and the
%                 round(((totalBoundingBox(ind(boxIdx),4)) + (totalBoundingBox(ind(boxIdx),3)))/2),...      % width and height, confidence, and
%                     round((totalBoundingBox(ind(boxIdx),2)) - (totalBoundingBox(ind(boxIdx),1))),...      % angle for each detection in order
%                         round((totalBoundingBox(ind(boxIdx),4)) - (totalBoundingBox(ind(boxIdx),3))));  % of highest confidence
%     end

%     for boxIdx = 1:sumbBoxes   % for testing
% %         bbox = [((boundingBox{j}(1) + boundingBox{j}(2))/2),... 
% %                     ((boundingBox{j}(3) + boundingBox{j}(4))/2),...
% %                     (boundingBox{j}(2) - boundingBox{j}(1)),...
% %                     (boundingBox{j}(4) - boundingBox{j}(3))];
%         bbox = [boundingBox{j}(1),... 
%                    boundingBox{j}(3),...
%                    (boundingBox{j}(2) - boundingBox{j}(1)),...
%                    (boundingBox{j}(4) - boundingBox{j}(3))];
% %         bbox = boundingBox{j};
%         fprintf(fileID,'%3.1f %3.1f %3.1f %3.1f %3.1f %d\n',...                                    % prints x and y coordinate of the
%             bbox, sortedBoxScores,angles);    % of highest confidence
%     end
% -----------------------------------------------------------------------------
%%    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % VISUALIZATION
    figure,imshow(Img);
     for j=1:Nclasses-1,
       sb = find(boxScores{j}>scoreAxis(20)); % show bounding boxes with a high score.
       plotBoundingBoxColor(boundingBox{j}(sb,:), colors(j), fix(boxScores{j}(sb)/40)+1); 
     end       
    
    str = num2str(i+1000);
    filename=strcat('figures/avi_MediaAnalyst_Proto2/',str,'.jpg');
    saveas(gcf,filename, 'jpg');
    close;
    ii=ii+1;
end
fclose(fileID);
save (dataFile, 'data');
save (detectFile,'detection');