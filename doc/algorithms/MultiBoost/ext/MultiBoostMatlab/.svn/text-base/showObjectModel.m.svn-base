% This script uses the parameters of the boosted detector and visualizes
% a part based model of the object by ploting the features used by the
% classifier.


clear all

parameters

% Load detector parameters:
load (dataFile)
NweakClassifiers = length(data.detector);

object = zeros(normalizedObjectSize+40); 
counts = zeros(normalizedObjectSize+40); 
[no mo] = size(object); cy = fix(no/2); cx = fix(mo/2);

figure
for j = 1:NweakClassifiers
    f = data.detector(j+46).featureNdx;
    
    feat = data.dictionary.filter{f};
    part = data.dictionary.patch{f};
    gx = data.dictionary.location{f}{1};
    gy = data.dictionary.location{f}{2};
    
    [foo, x] = max(gx);
    [foo, y] = max(gy);
    [n m] = size(part); n = (n-1)/2; m = (m-1)/2;
    
    x = (length(gx)+1)/2 - x + cx; 
    y = (length(gy)+1)/2 - y + cy;
    
    part = part-min(part(:));
    part = part/max(part(:));
    object(y-n:y+n, x-m:x+m) = object(y-n:y+n, x-m:x+m) + part;
    counts(y-n:y+n, x-m:x+m) = counts(y-n:y+n, x-m:x+m) + 1;

    if j < 19
        location = zeros(normalizedObjectSize+40);
        location (y,x) = 1; location = conv2(locSigma, locSigma, location, 'same');
        subplot(6,9,3*j-2);
        imagesc(zeroPad(feat, [11 11])); axis('equal'); colormap(gray(256)); axis('off'); axis('tight')
        subplot(6,9,3*j-1);
        imagesc(part); axis('equal'); colormap(gray(256)); axis('off'); axis('tight')
        subplot(6,9,3*j);
        imagesc(location); axis('equal'); colormap(gray(256)); axis('off'); axis('tight')
    end
end
counts(counts==0)=1;

Hfig = figure;
imagesc(object ./ counts); axis('equal')
colormap(gray(256))


