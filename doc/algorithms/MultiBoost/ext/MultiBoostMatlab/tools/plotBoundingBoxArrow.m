function plotBoundingBoxArrow(boundingBox,str,w,angulo)
%
% Plot bounding boxes. The thinkness of the line can be changed according
% to confidence.

Nboxes = size(boundingBox,1);

if nargin == 2
    w = ones(Nboxes,1)*2;
end

hold on

if angulo==0,
    for i = 1:Nboxes
        plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2) (boundingBox(i,1)+boundingBox(i,2))/2 boundingBox(i,1) boundingBox(i,1)], ...
            [boundingBox(i,3) boundingBox(i,3) boundingBox(i,4)  boundingBox(i,4)+(boundingBox(i,4)-boundingBox(i,3))/4  boundingBox(i,4) boundingBox(i,3)], ...
            'k', 'linewidth', w(i)+2)
        plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2) (boundingBox(i,1)+boundingBox(i,2))/2 boundingBox(i,1) boundingBox(i,1)], ...
            [boundingBox(i,3) boundingBox(i,3) boundingBox(i,4) boundingBox(i,4)+(boundingBox(i,4)-boundingBox(i,3))/4  boundingBox(i,4) boundingBox(i,3)], ...
            str, 'linewidth', w(i))
    end
end

if angulo==90,
    for i = 1:Nboxes
        plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2) boundingBox(i,1) boundingBox(i,1)-(boundingBox(i,2)-boundingBox(i,1))/3 boundingBox(i,1)], ...
            [boundingBox(i,3) boundingBox(i,3) boundingBox(i,4) boundingBox(i,4) (boundingBox(i,4)+boundingBox(i,3))/2 boundingBox(i,3)], ...
            'k', 'linewidth', w(i)+2)
        plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2) boundingBox(i,1) boundingBox(i,1)-(boundingBox(i,2)-boundingBox(i,1))/3 boundingBox(i,1)], ...
            [boundingBox(i,3) boundingBox(i,3) boundingBox(i,4) boundingBox(i,4) (boundingBox(i,4)+boundingBox(i,3))/2 boundingBox(i,3)], ...
            str, 'linewidth', w(i))
    end
end

if angulo==270,
    for i = 1:Nboxes
        plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2)+(boundingBox(i,2)-boundingBox(i,1))/3 boundingBox(i,2) boundingBox(i,1) boundingBox(i,1)], ...
            [boundingBox(i,3) boundingBox(i,3) (boundingBox(i,4)+boundingBox(i,3))/2 boundingBox(i,4) boundingBox(i,4) boundingBox(i,3)], ...
            'k', 'linewidth', w(i)+2)
        plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2)+(boundingBox(i,2)-boundingBox(i,1))/3 boundingBox(i,2) boundingBox(i,1) boundingBox(i,1)], ...
             [boundingBox(i,3) boundingBox(i,3) (boundingBox(i,4)+boundingBox(i,3))/2 boundingBox(i,4) boundingBox(i,4) boundingBox(i,3)], ...
            str, 'linewidth', w(i))
    end
end

if angulo==180,
    for i = 1:Nboxes
        plot([boundingBox(i,1) (boundingBox(i,1)+boundingBox(i,2))/2  boundingBox(i,2) boundingBox(i,2) boundingBox(i,1) boundingBox(i,1)], ...
            [boundingBox(i,3) boundingBox(i,3)-(boundingBox(i,4)-boundingBox(i,3))/4 boundingBox(i,3) boundingBox(i,4) boundingBox(i,4) boundingBox(i,3)], ...
            'k', 'linewidth', w(i)+2)
        plot([boundingBox(i,1) (boundingBox(i,1)+boundingBox(i,2))/2  boundingBox(i,2) boundingBox(i,2) boundingBox(i,1) boundingBox(i,1)], ...
            [boundingBox(i,3) boundingBox(i,3)-(boundingBox(i,4)-boundingBox(i,3))/4 boundingBox(i,3) boundingBox(i,4) boundingBox(i,4) boundingBox(i,3)], ...
            str, 'linewidth', w(i))
    end
end