readerobj = mmreader('D:\MyWorks\segments\MOV007_seg1.00.avi','tag', 'myreader1');
starto=0;
endo=169;
NtestImages=endo-starto;
% Loop on test images
ii=1;
Nclasses=5;
colors = 'rgbym';

figure;
for i = 1:NtestImages,
    vidFrame = read(readerobj,i+starto);  
    Img=vidFrame;
    img=Img;
    [rows cols dim]=size(img);
    
    nMarines=3;
    
     imshow(img);
    %DrawGT(img, nMarines, Nclasses,'torso_marine','image',[],ii);  
    DrawGT(img, nMarines, Nclasses,'annot/torso_marine','image','marine_',colors(5),ii); 
    
    DrawGT(img, nMarines, Nclasses,'annot/head_marine','image',[],colors(3),ii); 
    
   
    
    str = num2str(ii+1000);
    filename=strcat('figures/GT/',str,'.jpg');
    saveas(gcf,filename, 'jpg');
    ii=ii+1;
end

%aviobj = close(aviobj);