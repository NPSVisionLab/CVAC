function [boxes,scores,clases] = GatherDetections(boxes,scores,clases,k,min_neighbors)
% This function group the retrieved reponses and group them. Overlapping windows are grouped so the new window is the average of the overlaped windows.
% Boxes -  are the detections bounding boxes.
% scores - are the score vector, one entry for each box
% clases - a vector of classes. If entry j=K tells the box in the j place
% belongs to class K.
% k - scale factor used for image resize step
% min_neighbors - minimum neighbors. This will filter out all the detection that has
% less neighbors than that.
% Use
% [boxes,scores] = GatherDetections(boxes,scores,clases,1.25,1)



%min_neighbors=1;
k=1/k; %scale factor

%boxes=boxes(scores>200,:); % Pick only a few rectangles to see if this works
%boxes=boxes(find(boxes(:,1)>900),:);
%figure, plotBoundingBoxColor(boxes,'g');

if( min_neighbors ~= 0 ),
    % group retrieved rectangles in order to filter out noise 
    [ncomp,idx_seq] = SeqPartition(boxes,scores,k);
    
%     [comps(1:ncomp+1).neighbors] = deal(0);
%     [comps.x] = deal(0);
%     [comps.y] = deal(0);
%     [comps.width] = deal(0);
%     [comps.height] = deal(0);
    
    comps=zeros(ncomp,7);
    % ************  Explanation ***************** 
    % neighbors=comps(1)
    % rect.x=comps(2)
    % rect.y=comps(3)
    % rect.width=comps(4)
    % rect.height=comps(5)
    % rect.score=comps(6)
    % clase=comps(7)
    % *********************************************
    
    %count number of neighbors
        for i = 1:size(boxes,1)
           r1=boxes(i,:);
           idx=idx_seq(i)+1;
           comps(idx,1)=comps(idx,1)+1; 
           comps(idx,2)=comps(idx,2)+ r1(1);
           comps(idx,3)=comps(idx,3)+ r1(3);
           comps(idx,4)=comps(idx,4)+ r1(2)-r1(1);
           comps(idx,5)=comps(idx,5)+ r1(4)-r1(3);
           % We plan to take the average of the scores of the combined
           % detection, however it may be a good idea to pick the MAX.
           cmpa=[comps(idx,6) scores(i)];
           [val ind]=max(cmpa); % maximum score between stored combined rectangle and new one
           comps(idx,6)=val;
           clsa=[comps(idx,7) clases(i)]; % pick the class of the maximum score
           comps(idx,7)=clsa(ind);
           
           %comps(idx,6)=max(comps(idx,6),scores(i));  
        end
         
    % caculate average bounding box
        compt=[];
        for i = 1:ncomp,
            n = comps(i,1);
            if n >= min_neighbors,
                comp.x = (comps(i,2)*2 + n)/(2*n);
                comp.y = (comps(i,3)*2 + n)/(2*n);
                comp.width = (comps(i,4)*2 + n)/(2*n);
                comp.height = (comps(i,5)*2 + n)/(2*n);
                comp.neighbors = comps(i,1);
                comp.score = comps(i,6);
                comp.class = comps(i,7);
                % If you want the score to be the average use :
                % comp.score = (comps(i,6)*2+n)/(2*n);
                compt=[compt; comp.neighbors comp.x comp.y comp.width comp.height comp.score comp.class];
            end
        end
        
        % filter out small object rectangles inside large object rectangles
        result_seq=[];
        score_seq=[];
        class_seq=[];
        for i = 1: size(compt,1),
            r1 = compt(i,:);
            flag = 1;

            for j = 1: size(compt,1),
                r2 = compt(j,:);
                distanX = r2(4)* 0.2;
                distanY = r2(5)* 0.2;
                if (i ~= j && ...
                    r1(2) >= r2(2) - distanX && ...
                    r1(3) >= r2(3) - distanY && ...
                    r1(2) + r1(4) <= r2(2) + r2(4) + distanX && ...
                    r1(3) + r1(5) <= r2(3) + r2(5) + distanY && ...
                    (r2(1) > max( 3, r1(1)) || r1(1) < 3))
                    % We will not do this by now. Tested and is not a good
                    % idea.
                    flag = 0;
                    break;
                end
            end

            if( flag )
                result_seq=[result_seq; r1(2) r1(4)+r1(2) r1(3) r1(5)+r1(3)];
                score_seq=[score_seq; r1(6)];
                class_seq=[class_seq; r1(7)];
            end
        end
% Need to Chnge oreder of columns 
boxes=result_seq;
scores=score_seq;
clases=class_seq;
end




