
objects.name{1} = 'skull+az0deg';
objects.name{2} = 'skull+az90deg';
objects.name{3} = 'skull+az270deg';
objects.name{4} = 'skull+az180deg';


objetos = 'skull';


paramfile = 'demoHead';
paramfile2 = 'detecthead';
% DATABASES FOLDERS:
% Define the root folder for the images and annotations: 

HOMEIMAGES = 'C:\POSTDOC\LabelMe\Images'; 
HOMEANNOTATIONS = 'C:\POSTDOC\LabelMe\Annotations'; 

% Negative set of annotations
NEGIMAGES = 'C:\POSTDOC\LabelMe_Negatives\Images'
NEGANNOTATIONS = 'C:\POSTDOC\LabelMe_Negatives\Annotations'

% PARAMETERS TOY DATA:
Nclasses = 5;
% Create sharing matrix (each column is one node). 
%T = [1 0 0 0; 0 1 0 0; 0 0 1 0; 1 1 0; 1 0 1; 0 1 1; 1 1 1]'; % this for 3 classes plus background
nnodes=2^(Nclasses-1)-1;
T=dec2bin(1:nnodes,Nclasses-1);
T=fliplr(T)';
Ts = int2str(T);
T=str2num(Ts);
T=T-min(min(T));
% to remove sharing use:
% T = eye(Nclasses-1);

% PARAMETERS BOOSTING
Nnodes = size(T,2);

% LEARNING CLASSIFIER
Nthresholds = 105;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% FEATURES parameters

normalizedObjectSize = [25 25]; % size of object in a normalized frame (= [max height, max width])

% define a hand-picked set of filters to extract features from images:
filters = {[1], ...                  % original image
    [1 2 1;0 0 0; -1 -2 -1]/2, ...   % y derivative
    [1 0 -1; 2 0 -2; 1 0 -1]/2, ...  % x derivative
    [-1 -1 -1; -1 8 -1; -1 -1 -1]}; %, ...  % laplacian
    %[-0.0046   -0.3032   -0.0171; 0.1751   -0.0000   -0.1751; 0.0171    0.3032    0.0046],...    % orientation filter
    %[0.0171    0.1751   -0.0046; 0.3032    0.0000   -0.3032; 0.0046   -0.1751   -0.0171]};        % orientation filter
    
patchSize = [5:2:15]; % size of patch templates

sampleFromImages = 8; % Number of images used to build the dictionary of patches. The images selected will not be used for training or test.
patchesFromExample = 20; % Number of patches to be extracted from every image.
locSigma = exp(-(-3:3).^2/3^2); % spatial filtering of the correlation score.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% parameters for precomputed features
trainingImageSize = [200 200];  % size of the images used for training
negativeSamplesPerImage = 15;    % number of background samples extracted from each image
%NsamplesPerClass = 200;         % number of training images
testImageSize = [256 256];      % size of the images used for test
negImageSize = [128 128];      % size of the images used as negatives
dictionaryImageSize = fix(normalizedObjectSize*1.2);   % size of the images used as for dicitonary (s;ioghlty bigger than the silhuette)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Training parameters
%numTrainImages  = 200;        % number of object training instances, per category
numTrainImages  = 70;        % number of object training instances, per category
%NweakClassifiers = 150;  % Rounds of the boosting
NweakClassifiers = 200;  % Rounds of the boosting
numTestImages = 50;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Compute some common variables:
Nfilters = length(filters);
Npatches = patchesFromExample * Nfilters;
dataFile = fullfile('data', paramfile);
detectFile = fullfile('data', paramfile2);




