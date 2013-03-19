clear all

% load parameters for torso detection
parameters_torso;
load (detectFile);
detection_torso=detection;

% load parameters for head detection
parameters_head;
load (detectFile);
detection_head=detection;

scoreAxis = linspace(0, 300, 60);
k=0.8 % scaling factor
min_neighbors=1; % minimum neighbohors.
colors = 'rgby';
angulos=[0 90 270 180];

readerobj = mmreader('D:\MyWorks\segments\MOV01B_seg.avi','tag', 'myreader1');
starto=0;
%starto=21424;
%endo=21664;
%starto=1;
endo=200;
%movie(mov, 1, readerobj.FrameRate);
NtestImages=endo-starto;
% Loop on test images
ii=1;
figure;

for i = 1:NtestImages,
      vidFrame = read(readerobj,i+starto);  
    % Read image
    %vidFrame=imresize(vidFrame,[480 800]);
    %Img=imresize( vidFrame, 0.7, 'bilinear')'
    Img=vidFrame;
    img=Img(95:671,1:1024,1:3);
    %Img=imresize( vidFrame, 0.35, 'bilinear'); 
    %Img=Img(100:438,200:600,:);
    %Img=Img(34:235,:,:);
    %img=Img; 
    [rows cols dim]=size(img);

    for j=1:Nclasses-1,
       boundingBox{j}= detection_torso{ii}.boundingBox{j}; 
       boxScores{j}=detection_torso{ii}.boxScores{j}; 
    end
        
    
    imshow(img);
    
    boundingBoxto=[];
    boxScoresto=[];
    clases=[];

    for j=1:Nclasses-1,
        sb = find(boxScores{j}>scoreAxis(18)); % show bounding boxes with a high score.
        %plotBoundingBoxColor(boundingBox{j}(sb,:), colors(j), fix(boxScores{j}(sb)/40)+1); 
        % get the boxes regardless the class, the image, upper certain
        % a high score.
        boundingBoxto=[boundingBoxto;boundingBox{j}(sb,:)];
        boxScoresto=[boxScoresto;boxScores{j}(sb)];
        clases=[clases; (ones(length(sb),1)*j)];
    end


%     imshow(Img);
     [boxes,scores,clases] = GatherDetections(boundingBoxto,boxScoresto,clases,k,min_neighbors); % HERE IS THE GROUPING OF THE RESULTS 
     for kk=1:size(boxes,1)
       %plotBoundingBoxColor(boxes(kk,:), colors(clases(kk)) , fix(scores(kk,:)/40)+1);    
       %plotBoundingBoxArrow(boxes(kk,:), colors(1) , (fix(scores(kk,:)/40)+1),angulos(clases(kk)));  
       plotBoundingBoxLine(boxes(kk,:), colors(1) , (fix(scores(kk,:)/40)+1),angulos(clases(kk)));  
       %PlotCompass(boxes(kk,:),angulos(clases(kk)));
     end
     
     result_torso.boxes{i}=boxes;
     result_torso.scores{i}=scores;
     result_torso.clases{i}=clases;
     
     boxes(:,3:4)=boxes(:,3:4)+94; % because we chopped the black stripes of the image. This returns to original    
     nMarines=3;    
     [cfmi, totali, retrivedi, detectedi, FAi] = GetCFM(nMarines, Nclasses,'annot/torso_marine','image', boxes, clases, ii, [128 48]); % CREATES THE CONFUSION MATRIX FOR FRAME I
     
     boxes(:,3:4)=boxes(:,3:4)-94; % because we chopped the black stripes of the image. This returns to original    
     
     result_torso.cfm{i}=cfmi;
     result_torso.total{i}=totali;
     result_torso.retrived{i}=retrivedi;
     result_torso.detected{i}=detectedi;
     result_torso.FA{i}=FAi;

     %For each detected body check the head detector results and pick the
     %one that is the closest to the hypothetical head
     for j=1:Nclasses-1,
       boundingBox{j}= detection_head{ii}.boundingBox{j}; 
       boxScores{j}=detection_head{ii}.boxScores{j}; 
     end

    boundingBoxto=[];
    boxScoresto=[];
    clases=[];
    
     for j=1:Nclasses-1,
         sb = find(boxScores{j}>scoreAxis(3)); % show bounding boxes with a high score.
         boundingBoxto=[boundingBoxto;boundingBox{j}(sb,:)];
         boxScoresto=[boxScoresto;boxScores{j}(sb)];
         clases=[clases; (ones(length(sb),1)*j)];
     end

          
     sel_index=[];
     for kk=1:size(boxes,1), 
        index = PickClosestHead(boxes(kk,:),boundingBoxto);
        plotBoundingBoxLine(boundingBoxto(index,:), colors(2), fix(boxScoresto(index)/40)+1, angulos(clases(index)));    
        sel_index=[sel_index index];
     end
     
     boundingBoxto=boundingBoxto(sel_index,:);
     boxScoresto=boxScoresto(sel_index,:);
     clases=clases(sel_index,:);
     
     result_head.boxes{i}=boundingBoxto;
     result_head.scores{i}=boxScoresto;
     result_head.clases{i}=clases;
     
     boundingBoxto(:,3:4)=boundingBoxto(:,3:4)+94; % because we chopped the black stripes of the image. This returns to original
     nMarines=3;    
     [cfmi, totali, retrivedi, detectedi, FAi] = GetCFM(nMarines, Nclasses,'annot/head_marine','image', boundingBoxto, clases, ii, [25 25]); % CREATES THE CONFUSION MATRIX FOR FRAME I
     
     result_head.cfm{i}=cfmi;
     result_head.total{i}=totali;
     result_head.retrived{i}=retrivedi;
     result_head.detected{i}=detectedi;
     result_head.FA{i}=FAi;

    save confusion.mat result_head result_torso;
    str = num2str(i+1000);
    filename=strcat('figures/avi_group/',str,'.jpg');
    saveas(gcf,filename, 'jpg');
    ii=ii+1;
end

% This returns the total confusion matrix for marine 2
cf=zeros(4);
for i=1:200,
cf=cf+result_head.cfm{i}{2};
end

% This returns the total appearances for marine 2 (according to the GT)
to=0;
for i=1:200,
to=to+result_head.total{i}{2};
end

% This returns the total detections of marine 2 regardless its class
de=0;
for i=1:200,
de=de+result_head.detected{i}{2};
end