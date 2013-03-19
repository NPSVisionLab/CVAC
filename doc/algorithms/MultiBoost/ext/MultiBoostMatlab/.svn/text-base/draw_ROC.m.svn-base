clear all
vect_fp=[];
vect_tp=[];
parameters
load (dataFile)
testImageSize = [256 256];
leg1= 'Marine standing 0 degrees';
leg2= 'Marine standing 90 degrees';
leg3= 'Marine standing 270 degrees';


figure,hold on;
for kl=1: (Nclasses-1),
   
    RETREL=data.RETREL{kl};
    REL=data.REL{kl};
    RET=data.RET{kl};

    % Load detector parameters:

    NweakClassifiers = length(data.detector);
    plotstyle = {'ms-', 'go-', 'b^-'};

    [fp, tpr] = LMROC(RET(:,:,end), REL(:,:,end), RETREL(:,:,end));

    %tpr=[1;tpr;0]; % Adds a constant for the case that the threshold is zero and one
    %fpr=[1;fpr;0]; % Adds a constant for the case that the threshold is zero and one
    h(kl)=plot(fp(:,1), tpr(:,1), plotstyle{kl}, 'linewidth', 3);
    ylabel('Hits Rate'); xlabel('False Alarms Ratio')
    axis([0 max(fp) 0 1]); %axis('square'); 
    title('Detectors output');
    %set(gca,'XScale','log')
    %set(gca,'YScale','log')
    grid on    
    ind=find(fp<40);
    disp(sprintf('  performance on testing classifier k = %d false alarms = %1.2f, true hits = %1.2f \n', kl, fp(ind(1)), tpr(ind(1))))
    vect_fp=[vect_fp fp(ind(1))];
    vect_tp=[vect_tp tpr(ind(1))];
end
disp(sprintf('  performance on testing for false alarms = %1.2f, tp = %1.2f [%1.2f, %1.2f]', mean(vect_fp), mean(vect_tp), min(vect_tp), max(vect_tp)))    
legend(h, leg1, leg2, leg3, 'location', 'best');
 