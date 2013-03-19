function DrawGT(img, nMarines, Nclasses,ann_filename,img_literal,label,colo,ii)

    
% input parameters : nMarines, Nclasses, ann_filename, img_literal,
% boxes,clases, ii, size
% output arguments: cfm, total, retrived, detected, FA
%imshow(img);
angulos=[0 90 270 180];
colors = 'rgbym';

 for m =1:nMarines,
     for obj = 1:(Nclasses-1),
      % Evaluate performace looking at precision-recall with different                        
        text_name=strcat(ann_filename,num2str(m),'_',num2str(angulos(obj)),'.txt');                         
        img_name=strcat(img_literal,num2str(10000+ii-1),'.jpg');
    
         [numb_rect, rects] = read_file(text_name,img_name);

         for kk=1:size(rects,1)
             plotBoundingBoxLine(rects(kk,:), colo , 2 , angulos(obj));   
             recto=rects(kk,:);
             if ~isempty(label)
                  text(recto(1)-10,recto(3)-10,strcat(label,'_',num2str(m)));
             end
         end
         
         
              
     end

 
 end

 
end

