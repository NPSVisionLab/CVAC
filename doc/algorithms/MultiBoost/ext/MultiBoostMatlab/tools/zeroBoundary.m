function [im]=zeroBoundary(im,num)
% removes the border of the image and replaces it by zeros.
%

if ~exist('num')
  num=1;
end

im(:,1:num,:)=0;im(1:num,:)=0;
im(:,end-num+1:end,:)=0;im(end-num+1:end,:,:)=0;

