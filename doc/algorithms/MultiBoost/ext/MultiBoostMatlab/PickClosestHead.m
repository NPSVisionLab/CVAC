function index = PickClosestHead(box,boundingBoxto)
 %% HERE select THE HEAD USING REGIONS FROM THE BODIES. 
 % assume that the best head is in the middle of the width, and 1/3 from
 % the top
    
    %for every person detection, regardless the class try to find the head
    % in a limited area
    
   
     search_area=1/4;
    recto=box;
    widt=recto(2)-recto(1);
    heigt=recto(4)-recto(3);
    
    nHeads=size(boundingBoxto,1);
     
    Ehead_x=repmat(recto(1)+widt/2,nHeads,1);
    Ehead_y=repmat(recto(3)+heigt/9,nHeads,1);
    
    Rhead_x=boundingBoxto(:,1)+(boundingBoxto(:,2)-boundingBoxto(:,1))/2;
    Rhead_y=boundingBoxto(:,3)+(boundingBoxto(:,4)-boundingBoxto(:,3))/2;

    distan =  sqrt((Ehead_x-Rhead_x).^2+(Ehead_y-Rhead_y).^2);
    [dis index]=min(distan);
    
    if (dis>widt/3),
       index=[]; 
    end   
end


    