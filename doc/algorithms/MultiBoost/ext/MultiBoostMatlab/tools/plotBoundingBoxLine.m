function plotBoundingBoxLine(boundingBox,str,w,angulo)
%
% Plot bounding boxes. The thinkness of the line can be changed according
% to confidence.

Nboxes = size(boundingBox,1);

if nargin == 2
    w = ones(Nboxes,1)*2;
end

hold on
for i = 1:Nboxes
    plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2) boundingBox(i,1) boundingBox(i,1)], ...
        [boundingBox(i,3) boundingBox(i,3) boundingBox(i,4) boundingBox(i,4) boundingBox(i,3)], ...
        'k--', 'linewidth', 2);
    plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2) boundingBox(i,1)  boundingBox(i,1)], ...
        [boundingBox(i,3) boundingBox(i,3) boundingBox(i,4) boundingBox(i,4) boundingBox(i,3)], ...
        [str '--'], 'linewidth', 1);
    width=boundingBox(i,2)-boundingBox(i,1);
    height=boundingBox(i,4)-boundingBox(i,3);
    cx=(boundingBox(i,1)+boundingBox(i,2))/2;
    cy=(boundingBox(i,3)+boundingBox(i,4))/2;
    y2=cy-1/2*height*(sin((270-angulo)*pi/180));
    x2=cx+1/2*width*(cos((270-angulo)*pi/180));
    plot([cx x2],[cy y2],str, 'linewidth', w(i));
    % the small corners of the arrow (this is not necessary)
    y3=cy-1/2*0.6*height*(sin((270-angulo-6)*pi/180));
    x3=cx+1/2*0.6*width*(cos((270-angulo-6)*pi/180));
    y4=cy-1/2*0.6*height*(sin((270-angulo+6)*pi/180));
    x4=cx+1/2*0.6*width*(cos((270-angulo+6)*pi/180));
    
    plot([x2 x3],[y2 y3],str, 'linewidth', w(i));
    plot([x2 x4],[y2 y4],str, 'linewidth', w(i));
    
end


