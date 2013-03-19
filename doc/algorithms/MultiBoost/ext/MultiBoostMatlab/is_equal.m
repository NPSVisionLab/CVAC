function [v] = is_equal(r1,r2,k)
% This function retrieves whether the two rectangles (r1 and r2) are equal.
% The multiplication factor k, is provided. This value depends on how we
% scale our detector between consecutive scales. If you scale it by sc=0.8,
% then k=1/sc, hence k=1.25.
% Note, in OpenCV this is fixed to be 1.2
distanX=r1(3)*0.2;
distanY=r1(4)*0.2;
v=  (r2(1)<=r1(1)+distanX) && ...
    (r2(1)>=r1(1)-distanX) && ...
    (r2(2)<=r1(2)+distanY)&& ...
    (r2(2)>=r1(2)-distanY) && ...
    (r2(3)<=r1(3)*k) && ...
    (r2(3)*k>=r1(3));
% 
% 
% static int is_equal( const void* _r1, const void* _r2, void* )
% {
%     const CvRect* r1 = (const CvRect*)_r1;
%     const CvRect* r2 = (const CvRect*)_r2;
%     int distance = cvRound(r1->width*0.2);
% 
%     return r2->x <= r1->x + distance &&
%            r2->x >= r1->x - distance &&
%            r2->y <= r1->y + distance &&
%            r2->y >= r1->y - distance &&
%            r2->width <= cvRound( r1->width * 1.2 ) &&
%            cvRound( r2->width * 1.2 ) >= r1->width;
% }