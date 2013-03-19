function [Score, boundingBox, boxScores] = mHaar(XMLFLN, RGB,SF,MN,WH)
%
%
% 1) To Run the Haar detector using the XML classifier in the given
%    filename, and a given color image
%     [Score, boundingBox, boxScores]=mHaar(XMLFLN, RGB,SF,MN,WDT, HGT );
%       
%       Input parameters
%       XMLFLN = Filename of the XML file for the detector
%       RGB = Color image of type double
%       SF = Scale factor. Usually 1.1
%       MN = Minimum neighbors. Usually 3
%       WH = Width and Height of the minimum detection window. Can be 24 pixels
% 
%        Output Parameters
%        Score = Score matrix, for Haar will be empty so far (until we fix this)
%        boundingBox = Bounding box on the format [x1,x1+width,y1,y1+height]
%        boxScores = Scores of each box detected. So far we will give to all of them 100. (sometime we will fix this). 
%
%      e.g.
%     
%
%    fn1='C:\Program Files\OpenCV\data\haarcascades\haarcascade_frontalface_alt.xml';
%    rgb=imread('C:\Program Files\OpenCV\samples\c\lena.jpg');
%    img = double(img);
%    [Score, boundingBox, boxScores]=mHaar(fn1,img,1.1,3, [24 24]);
WDT=WH(1);
HGT=WH(2);
Bbox=Haar(XMLFLN, RGB,SF,MN,WDT, HGT );
boundingBox=[Bbox(:,1) Bbox(:,1)+Bbox(:,3) Bbox(:,2) Bbox(:,2)+Bbox(:,4)];
Score=[];
boxScores=ones(size(boundingBox,1),1)*100;


