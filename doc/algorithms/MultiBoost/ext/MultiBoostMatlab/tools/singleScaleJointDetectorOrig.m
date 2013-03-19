function [Score, boundingBox, boxScores] = singleScaleJointDetector(img, data, T)
%
% This runs the detector at single scale.
%
% In order to build a multiscale detectors, you need to loop on scales.
% Something like this:
% for scale = 1:Nscales
%    img = imresize(img, .8, 'bilinear');
%    Score{scale} = singleScaleBoostedDetector(img, data);
% end

[Nclasses, Nnodes] = size(T);
[rows,cols, Nsamples] = size(img); % Nsamples = Number of thresholds that we will consider

nodes = [data.detector(:).bestnode];

% 1) compute each node-function
%Fn = zeros(Nsamples, Nnodes); % F(n) is the function in node 'n'. 
Fn = zeros(rows,cols, Nnodes); % F(n) is the function in node 'n'. 

if nargout == 4
    Nstumps = length(data.detector);
    Fstumps = zeros(rows,cols, Nstumps);
    stump = 0;
end

NweakClassifiers = length(data.detector);

% Initialize variables
Score = 0; nwScore = 0;

%Each function in Fn is computed by adding the corresponding boosting rounds.
for n = 1:Nnodes
    stumpsInNode = find(nodes == n); % this gives the set of all stumps that correspond to F(n) 
    for m = stumpsInNode
        k = data.detector(m).featureNdx;
        feature = convCrossConv(img, data.dictionary.filter(k), data.dictionary.patch(k), data.dictionary.location(k));
        
        th = data.detector(m).th;
        a = data.detector(m).a;
        b = data.detector(m).b;
     
        fm = (a * (feature > th) + b); % regression stump
        Fn(:,:, n) = Fn(:,:, n) + fm(:,:);  % Calculate G1(v), G2(v), G3(v) , G23(v) , G13(v) + G123(v) 
        
        if nargout == 4
            stump = stump + 1;
            Fstumps(:,:, stump) = fm';
        end
    end
end

% 2) compute the final classifiers from the node-functions by applying the
% necessary combinations in order to get the classifiers corresponding at
% each class.
Fx = zeros(rows,cols, Nclasses);
for i = 1:Nclasses
    j = find(T(i,:)); % nodes conected to each classifier.
    Fx(:,:,i) = sum(Fn(:,:,j),3) + data.detector(1).b0(i); %Calculate H(v,1), H(v,2) and H(v,3). Remember H(v,1)=G123(v)+G12(v)+G2(v). 
end

nDetections=[];

for k = 1:Nclasses, 
    Score=Fx(:,:,k);
    % Look at local maximum of output score and output a set of detected object
    % bounding boxes.
    s = double(Score>0);
    %s = conv2(hamming(35),hamming(35),s,'same');
    s = conv2(hamming(35),hamming(35),s,'same');  % Modified by Juan
     BW = imregionalmax(s);
    [y, x] = find(BW.*s);

    D = distance([x y]',[x y]'); D = D + 1000*eye(size(D));
    while min(D(:))<10
        N = length(x);
        [i,j] = find(D==min(D(:)));
        x(i(1)) = round((x(i(1)) + x(j(1)))/2);
        y(i(1)) = round((y(i(1)) + y(j(1)))/2);
        x = x(setdiff(1:N,j(1)));
        y = y(setdiff(1:N,j(1)));
        D = distance([x y]',[x y]'); D = D + 1000*eye(size(D));
    end

    nDetections{k} = length(x);
    boundingBox{k} = repmat(data.averageBoundingBox{k}, [nDetections{k} 1]) + [x x y y];
    ind = sub2ind(size(s), y, x);
    boxScores{k} = s(ind);
end