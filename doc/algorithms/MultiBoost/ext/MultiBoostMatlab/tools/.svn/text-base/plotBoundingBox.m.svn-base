function plotBoundingBox(boundingBox, w)
%
% Plot bounding boxes. The thinkness of the line can be changed according
% to confidence.

Nboxes = size(boundingBox,1);

if nargin == 1
    w = ones(Nboxes,1)*2;
end

hold on
for i = 1:Nboxes
    plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2) boundingBox(i,1) boundingBox(i,1)], ...
        [boundingBox(i,3) boundingBox(i,3) boundingBox(i,4) boundingBox(i,4) boundingBox(i,3)], ...
        'k', 'linewidth', w(i)+2)
    plot([boundingBox(i,1) boundingBox(i,2) boundingBox(i,2) boundingBox(i,1)  boundingBox(i,1)], ...
        [boundingBox(i,3) boundingBox(i,3) boundingBox(i,4) boundingBox(i,4) boundingBox(i,3)], ...
        'r', 'linewidth', w(i))
end


