function [numb_rect, rects] = read_file(file_apples,literal)
%file_apples='C:\POST\methods\apple_places_IR.txt';
%literal='E0701_02.tif';
% output %number of detections
% output [x1,x2,y1,y2] per detection
% Open the file for read
detect_vect=[];
fid = fopen(file_apples, 'rt');
y = 0;
while feof(fid) == 0
   tline = fgetl(fid);
   % matches the each line of the data in the text file with the literal
   matches = findstr(tline, literal);
   num = length(matches);
   if num > 0
      y = y + num;
      %fprintf(1,'%d:%s\n',num,tline);
      % If there is a matching, keep the line stored
      mline=tline;
      break;
   end
end
fclose(fid);

if (num==0),
   numb_rect=0;
   rects=[];
   return;
end
% Take the line matched with the literal, in our case the literal is the
% filename
remain = mline;
contador=0;
while true
    % go over the line string and check every word in the string
   contador=contador+1;
   [str, remain] = strtok(remain);
   if isempty(str),  
       break;
   end
   %disp(sprintf('%s', str));
   % If is the second place (one after the filename), means that this is
   % the number of apples detected in the file
   if contador==2,
       numb_rect=str2num(str);
   end
   % The following values after that are all bouding box with a
   % x,y,with,height coordinates
   if contador>2
       % Kepp the data regarding all the bouding boxes in the vector called
       % detect_vect
      detect_vect(contador-2)=str2num(str); 
   end
end

rects=[];
contador=1;
for indice=1:4:length(detect_vect),
     x=detect_vect(indice);
     y=detect_vect(indice+1);
     width=detect_vect(indice+2);
     height=detect_vect(indice+3);
     x1=round(x-width/2);
     x2=round(x+width/2);
     y1=round(y-height/2);
     y2=round(y+height/2);
     rects=[rects;x1 x2 y1 y2];
     contador=contador+1;
end