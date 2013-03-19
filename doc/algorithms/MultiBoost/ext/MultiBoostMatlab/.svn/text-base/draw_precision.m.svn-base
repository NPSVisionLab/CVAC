clear all

parameters
load (dataFile)
testImageSize = [256 256];

RETREL=data.RETREL;
REL=data.REL;
RET=data.RET;

% Load detector parameters:

NweakClassifiers = length(data.detector);
plotstyle = {'rs-', 'go-', 'b^-'};

precision = squeeze(sum(RETREL,2) ./ sum(RET,2));
recall    = squeeze(sum(RETREL,2) ./ sum(REL,2));

recall=[1;recall;0]; % Adds a constant for the case that the threshold is zero and one
precision=[0;precision;1]; % Adds a constant for the case that the threshold is zero and one
plot(1-precision(:,1), recall(:,1), plotstyle{1}, 'linewidth', 3);
ylabel('recall'); xlabel('1-precision')
axis([0 1 0 1]); %axis('square'); 
grid on    