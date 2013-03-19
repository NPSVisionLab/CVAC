function [boxes,scores,clas] = DetectHeads(img,box)
 %% HERE DETECT THE HEADS USING REGIONS FROM THE BODIES. USE 1/4 TOP
    %% HALF OF THE BODY AND THE 1/4 OVER THE TOP OF THE BODY.
    %% CALL THE HAAR DETECTOR
    
    %for every person detection, regardless the class try to find the head
    % in a limited area
    %fn='C:\POSTDOC\Base_It\LM_Detection\Experiment12_5\cascades\headdet.xml';
    fn1='cascades\head_0.xml';
    fn2='cascades\head_90.xml';
    fn3='cascades\head_270.xml';
    fn4='cascades\head_180.xml';
       
    
    min_size=[20 20]; %minimum size of the head detector

    search_area=1/4;
    recto=box;
    widt=recto(2)-recto(1);
    heigt=recto(4)-recto(3);
    
    %if the size of the image is smaller than the head detector, leave
    if min(widt,heigt)<min(min_size)
       boxes=[];
       scores=[];
       clas=0;
       return;
    end

    recto(4)=recto(3)+heigt*search_area;
    recto(3)=recto(3)-heigt*search_area;
    new_width=recto(2)-recto(1);
    new_heigt=recto(4)-recto(3);
    
    if recto(3)<=0,
        recto(3)=1;
    end
        
    new_img=img(recto(3):recto(4),recto(1):recto(2),:);
    %figure,imshow(new_img);

    [Score, bB{1}, boxScores] = mHaar(fn1,double(new_img),1.1,1,[20 20]);
    [Score, bB{2}, boxScores] = mHaar(fn2,double(new_img),1.1,1,[20 20]);
    [Score, bB{3}, boxScores] = mHaar(fn3,double(new_img),1.1,1,[20 20]);
    [Score, bB{4}, boxScores] = mHaar(fn4,double(new_img),1.1,1,[20 20]);
    
    %lets find which detector got the most responses
    % (this is not the best way to find the winner, but is something)
    % also try the detector that gave the closest detection to the
    % predicted center of the head
    sz=[];
    for (i=1:4),
        nBox=size(bB{i},1);
        sz=[sz nBox];
    end
    
    [val ind]=max(sz);
            
     if val>0, %number of boxes
         clas=ind;
     else
         clas=0;
     end

     recto=[recto(1) recto(1) recto(3) recto(3)]; 
     add_origen=repmat(recto,val,1);
     
     boxes=bB{ind}+add_origen; % translate back to origen of image
     scores=Score;
     
end