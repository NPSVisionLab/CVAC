function [RET, REL, RETREL, ndx] = LMprecisionRecallText(retrievedBoundingBox, file_name,img_name,mindetectablesize)
% This function returns one point in the precision-recall curve.
% Provided a set of detected bounding boxes, it indicates how may objects
% have been detected (RETREL).
%
% The standard measures for IR are recall and precision. Assuming that:
%
%    * RET is the set of all items the system has retrieved for a specific
%    inquiry; (tp + fp)
%    * REL is the set of relevant items for a specific inquiry;  (tp+fn)
%    * RETREL is the set of the retrieved relevant items (tp) 
%
% then precision and recall measures are obtained as follows:
%
%    precision = RETREL / RET
%    recall = RETREL / REL 
%
%    false positive rate = fpr = fp / (fp + tn) = (RET-RETREL)/(RET-RETREL+tn) 
%    true postive rate =  tpr = recall
%


% Search the target object in the annotation


 [numb_rect, rects] = read_file(file_name,img_name);

  Ninstances = numb_rect;

% Extract the bounding boxes for each target present in the image
BoundingBox = []; REL = 0;
for i = 1:Ninstances
    
    BoundingBox = [BoundingBox; rects(i,1) rects(i,2) rects(i,3) rects(i,4)];
   
    % If we detect an object that we thought was too small, we should not
    % penalize performance for this. So, we will consider that the only
    % relevant targets are the ones with a size larger than the minimal
    % detectable size. 
    minY=mindetectablesize(1)*0.75;  % We allow an object to be recognized if it larger 75% of the min size
    minX=mindetectablesize(2)*0.75;
    %minY=mindetectablesize(1);  % 
    %minX=mindetectablesize(2);  %
    
    if (rects(i,4)-rects(i,3)>minY | rects(i,2)-rects(i,1)>minX) & rects(i,4)>0 & rects(i,2)>0
        REL = REL + 1;
    end
end

RET = size(retrievedBoundingBox, 1);

if REL == 0 
    RETREL = 0;
    ndx=[];
    return
end

cxO = mean(BoundingBox(:,1:2),2); % center x
cyO = mean(BoundingBox(:,3:4),2); % center y
DxO = diff(BoundingBox(:,1:2),1,2); % width
DyO = diff(BoundingBox(:,3:4),1,2); % height

cxR = mean(retrievedBoundingBox(:,1:2),2); % center x
cyR = mean(retrievedBoundingBox(:,3:4),2); % center y
DxR = diff(retrievedBoundingBox(:,1:2),1,2); % width
DyR = diff(retrievedBoundingBox(:,3:4),1,2);  % height

ndx = [];
for i = 1:RET
    d = sqrt(((cxR(i) - cxO)./DxO).^2 + ((cyR(i) - cyO)./DyO).^2)<.5 ...
        & max(DxO/DxR(i), DxR(i)./DxO)<1.5 ...
        & max(DyO/DyR(i), DyR(i)./DyO)<1.5;
    n = find(d);
    if length(n)>0
        ndx(i) = n(1);
    else
        ndx(i) = 0;
    end
end
RETREL = sum(unique(ndx)>0); % each object can be detected only once

% If we detect an object that we thought was too small, we should not
% penalize performance for this. So, if we detected one object that was not
% considered before within the relevan set, then we will move it into the
% relevant set:
REL = max(REL, RETREL);



