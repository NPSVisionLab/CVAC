clear all
fn1='C:\Program Files\OpenCV\data\haarcascades\haarcascade_frontalface_alt.xml';
fn2='C:\Program Files\OpenCV\samples\c\lena.jpg';
rgb=imread(fn2);
im=double(rgb);  % Must be of type double & color
Bbox=Haar(fn1,im,1.2,2,30,30);
figure,imshow(rgb);
% Convert to Matlab format
boundingBox=[Bbox(:,1) Bbox(:,1)+Bbox(:,3) Bbox(:,2) Bbox(:,2)+Bbox(:,4)];
plotBoundingBox(boundingBox);
drawnow;



