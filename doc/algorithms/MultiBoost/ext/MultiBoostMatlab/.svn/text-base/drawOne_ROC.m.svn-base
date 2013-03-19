clear all
vect_fp=[];
vect_tp=[];
parameters
load (dataFile);
load ('data/databaseStruct');
NweakClassifiers = length(data.detector);

testImageSize = [256 256];
leg= 'Marine multi-detector';
%NweakClassifiers = length(data.detector);
plotstyle = {'rs-', 'go-', 'b^-', 'm*-'};
%NweakClassifiers = [30 80 120 150];
%NweakClassifiers = [150];

RETREL=0;
RET=0;
REL=0;

figure,hold on;
for kl=1: (Nclasses-1),
    RETREL=RETREL+data.RETREL{kl};
    REL=REL+data.REL{kl};
    RET=RET+data.RET{kl};
end
    % Load detector parameters:

 for m = 1:length(NweakClassifiers),
        [fp, tpr] = LMROC(RET(:,:,m), REL(:,:,m), RETREL(:,:,m));
        h(m)=plot(fp(:,1), tpr(:,1), plotstyle{m}, 'linewidth', 3);hold on
      
 end
legend(h, strcat('Rounds',num2str(NweakClassifiers(:))), 'location', 'best')
ylabel('Hits Rate'); xlabel('False Alarms Ratio');
axis([0 max(fp) 0 1]); %axis('square'); 
grid on
    
    
   
ind=find(fp<40);
disp(sprintf('  performance on testing . false alarms = %1.2f, true hits = %1.2f \n',  fp(ind(1)), tpr(ind(1))))
vect_fp=[vect_fp fp(ind(1))];
vect_tp=[vect_tp tpr(ind(1))];

disp(sprintf('  performance on testing for false alarms = %1.2f, tp = %1.2f [%1.2f, %1.2f]', mean(vect_fp), mean(vect_tp), min(vect_tp), max(vect_tp)))    
%legend(h, leg, 'location', 'best');
 