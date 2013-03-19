

load confusion.mat;

cf1=zeros(4);
for i=1:169,
cf1=cf1+result_torso.cfm{i}{1};

    if (cf1(1,3)~=0)
          toto=1;
    end
end

cf2=zeros(4);
for i=1:169,
cf2=cf2+result_torso.cfm{i}{2};
end

% This returns the total appearances for marine 2 (according to the GT)
to=0;
for i=1:169,
to=to+result_torso.total{i}{2};
end

% This returns the total detections of marine 2 regardless its class
de=0;
for i=1:169,
de=de+result_torso.detected{i}{2};
end

DE1=[]; 
for i=1:169, 
     [st ind]=find(result_torso.cfm{i}{1}==1);
     if ~isempty(ind),
        DE1=[DE1 ind]; 
     else
         DE1=[DE1 0];
     end
end

DE2=[]; 
for i=1:169, 
       [st ind]=find(result_torso.cfm{i}{2}==1);
     if ~isempty(ind),
        DE2=[DE2 ind]; 
     else
         DE2=[DE2 0];
     end
end

DE3=[]; 
for i=1:169, 
     [st ind]=find(result_torso.cfm{i}{3}==1);
     if ~isempty(ind),
        DE3=[DE3 ind]; 
     else
         DE3=[DE3 0];
     end
end

GT1=[]; 
for i=1:169, 
     ind=find(result_torso.total{i}{1}==1);
     if ~isempty(ind),
        GT1=[GT1 ind]; 
     else
         GT1=[GT1 0];
     end
end

GT2=[]; 
for i=1:169, 
     ind=find(result_torso.total{i}{2}==1);
     if ~isempty(ind),
        GT2=[GT2 ind(1)]; 
     else
         GT2=[GT2 0];
     end
end

GT3=[]; 
for i=1:169, 
     ind=find(result_torso.total{i}{3}==1);
     if ~isempty(ind),
        GT3=[GT3 ind(1)]; 
     else
         GT3=[GT3 0];
     end
end