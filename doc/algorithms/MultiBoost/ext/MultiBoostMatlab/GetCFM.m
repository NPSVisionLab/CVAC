function [cfm, total, retrived, detected, FA] = GetCFM(nMarines, Nclasses,ann_filename,img_literal, boxes, clases, ii, sized)
% This function creates a ConFusion Matrix (CFM) per marine, per posture, per
% given frame ii.

% INPUT:
% nMarines = number of marines
% Nlcasses= number of postures + 1
% ann_filename = the proceeding filename, for e.g. "torso_marine". Since annotation file "torso_marine2_90.txt" is all the instances of marine 2 in 90 degrees in this frame
% img_literal = the proceeding filename inside the annot. text file, like %% "image" is for entry image10000.jpg and so on
% boxes= the boxes detected by the detector
% clases = class that each detection belongs to
% ii = number of frame
% sized = size of the minimum detection

% OUTPUT:
% cfm{i}: confusion matrix for marine i 
% total{i} ground true appearances in the image of marine i
% retrived {i}: all the detections retreived in the image
% detected{i}: the number of detections of the marine i, regardless its class
% FA : number of false alarms in the image.

% 
angulos=[0 90 270 180];
RET = []; REL = []; RETREL = [];
cmt=zeros(Nclasses-1,Nclasses-1);  %Create the confusion matrix. rows and cols are the claases, and the
tmp=zeros(Nclasses-1,1); 
 ndx=[];
 assigned=zeros(size(boxes,1),1);     
 
 for m =1:nMarines,
     cfm{m}=cmt; %zeros
     total{m}=tmp; % true total accoring to GT
     retrived{m}=tmp; % retreived by detector
     detected{m}=tmp; % a detection (regardless the class)

     for obj = 1:(Nclasses-1),
      % Evaluate performace looking at precision-recall with different
            % thresholds:
            n = 1; % this is the scoreAxis, we will use all the time 25
                % Now lets find the ROC plot 
                for kl=1: (Nclasses-1),
                        [ind val]=find(clases==kl & assigned==0);
                        boundedbox=boxes(ind,:);
                        text_name=strcat(ann_filename,num2str(m),'_',num2str(angulos(obj)),'.txt');                         
                        img_name=strcat(img_literal,num2str(10000+ii-1),'.jpg');
                        [RET{m}{kl}, REL{m}{kl}, RETREL{m}{kl} ndx] = LMprecisionRecallText(boundedbox, text_name,img_name,sized);       
                         % Create confusion matrix for image i
                         cfm{m}(obj,kl)=RETREL{m}{kl}; %true positives detected (confusion matrix)     

                         % increment the total number of GT and the
                         % detected 
                         if kl==obj, 
                             total{m}(kl)=REL{m}{kl}+total{m}(kl);     % total marines according to GT             
                             retrived{m}(kl)=length(find(clases==kl));
                         else
                          % If this detector is different than the class checked now
                             RETREL{m}{kl}=0;  %Make the true hits be 0 since this class do not correspond to the one tested 
                             REL{m}{kl}=0; %Make the targets be 0 since this class do not correspond to the one supplied 
                         end

                         %mark the assigned box
                         if ~isempty(ndx)
                            assigned(ind)=ndx;
                         end

                end
     end

     detected{m}=sum(cfm{m},2)'; %total detection for each class (regardless correct or not), for frame ii
 end

 cf=zeros(4);
 for m =1:nMarines, cf=cf+cfm{m};end    
 FA=sum(retrived{1}(:))-sum(sum(cf)); % false alarms per frame ii
 
end

