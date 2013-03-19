

load confusion.mat;
nMarines=3;
nFrames=200;

for j=1:nMarines,
    cf{j}=zeros(4);
    for i=1:nFrames,
        cf{j}=cf{j}+result_torso.cfm{i}{j};
    end
end

% This returns the total appearances for marine 2 (according to the GT)
for j=1:nMarines,
    to{j}=0;
    for i=1:nFrames,
        to{j}=to{j}+result_torso.total{i}{j};
    end
end

% This returns the total detections of marine 2 regardless its class
for j=1:nMarines,
    de{j}=0;
    for i=1:nFrames,
        de{j}=de{j}+result_torso.detected{i}{j}';
    end
end

FA=0;
for i=1:nFrames,
    FA=FA+result_torso.FA{i};
end


% SEQUENCES
% *************************************************************************

for j=1:nMarines,
    DE{j}=[]; 
    for i=1:nFrames, 
         [st ind]=find(result_torso.cfm{i}{j}==1);
         if ~isempty(ind),
            DE{j}=[DE{j};ind]; 
         else
             DE{j}=[DE{j};0];
         end
    end
end

for j=1:nMarines,
    GT{j}=[]; 
    for i=1:nFrames, 
         ind=find(result_torso.total{i}{j}==1);
         if ~isempty(ind),
            GT{j}=[GT{j};ind]; 
         else
             GT{j}=[GT{j};0];
         end
    end
end
