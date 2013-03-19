clear all

parameters
load (detectFile);
%load ('data/databaseStruct');
scoreAxis = linspace(0, 300, 60);
k=0.8 % scaling factor
min_neighbors=1; % minimum neighbohors.
colors = 'rgby';
angulos=[0 90 270 180];

readerobj = mmreader('D:\MyWorks\segments\MOV007_seg1.00.avi','tag', 'myreader1');
starto=0;
endo=169;
NtestImages=endo-starto;
% Loop on test images
ii=1;
figure;

for i = 1:NtestImages,
      vidFrame = read(readerobj,i+starto);  
    % Read image
    Img=vidFrame;
    img=Img(95:671,1:1024,1:3);
    %Img=imresize( vidFrame, 0.35, 'bilinear');  
    [rows cols dim]=size(img);

    for j=1:Nclasses-1,
       boundingBox{j}= detection{ii}.boundingBox{j}; 
       boxScores{j}=detection{ii}.boxScores{j}; 
    end
        
    
    imshow(img);
    
    boundingBoxto=[];
    boxScoresto=[];
    clases=[];

    for j=1:Nclasses-1,
        sb = find(boxScores{j}>scoreAxis(25)); % show bounding boxes with a high score.
        %plotBoundingBoxColor(boundingBox{j}(sb,:), colors(j), fix(boxScores{j}(sb)/40)+1); 
        % get the boxes regardless the class, the image, upper certain
        % a high score.
        boundingBoxto=[boundingBoxto;boundingBox{j}(sb,:)];
        boxScoresto=[boxScoresto;boxScores{j}(sb)];
        clases=[clases; (ones(length(sb),1)*j)];
    end

%     imshow(Img);
     % HERE IS THE GROUPING OF THE RESULTS 
     [boxes,scores,clases] = GatherDetections(boundingBoxto,boxScoresto,clases,k,min_neighbors); 
     for kk=1:size(boxes,1) 
       plotBoundingBoxLine(boxes(kk,:), colors(1) , (fix(scores(kk,:)/40)+1),angulos(clases(kk)));  
     end
           
     %For each detected body call a head detector (with orientations)
     for kk=1:size(boxes,1), 
        [box,score,clas] = DetectHeads(img,boxes(kk,:));
        if clas>0,
            for jj=1:size(box,1)
                plotBoundingBoxArrow(box(jj,:), colors(2) , 1,angulos(clas));  
            end
        end
     end
    
    str = num2str(ii+1000);
    filename=strcat('figures/avi_group/',str,'.jpg');
    %saveas(gcf,filename, 'jpg');
    ii=ii+1;
end

%aviobj = close(aviobj);