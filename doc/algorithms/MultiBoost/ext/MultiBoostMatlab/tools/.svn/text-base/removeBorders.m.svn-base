function [newImg,newAnn]=removeBorders(Img,annotation)
% removes the border of the image if it is all black
% and updates the annotation (newannotation) with the new (smaller) image
% Use :  [newImg,newAnn]=removeBorders(Img,annotation)
if (mean(mean(Img(1:10,:,:))) + mean(mean(Img(end-10:end,:,:))))==0,
   avg=mean(Img,2);  % Average the image over the second dimension\
   ind=find(avg>0);
   bord=ind(1);
   newImg=Img(bord:end-bord,:,:);
   newAnn=annotation;
   Nobjects=length(annotation.object);
   for i=1:Nobjects,
        Npoints = length(annotation.object(i).polygon.pt);
        clear X Y
        for j = 1:Npoints
            % Scale each point
            y=str2num(annotation.object(i).polygon.pt(j).y);
            Y(j) = round(y - bord);
            newAnn.object(i).polygon.pt(j).y = num2str(Y(j));
        end 
   end
   
end
