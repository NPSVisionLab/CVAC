function [Score, boundingBox, boxScores] = singleScaleJointDetector(img, data, T, NweakClassifiers)
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

%NweakClassifiers = length(data.detector);
data.detector=data.detector(1:NweakClassifiers);

nodes = [data.detector(:).bestnode];

% 1) compute each node-function
%Fn = zeros(Nsamples, Nnodes); % F(n) is the function in node 'n'. 
Fn = zeros(rows,cols, Nnodes); % F(n) is the function in node 'n'. 

if nargout == 4
    Nstumps = length(data.detector);
    Fstumps = zeros(rows,cols, Nstumps);
    stump = 0;
end

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
yt=[];
xt=[];
kt=[];
% The follow part was modified from the original. The reason is that we
% will present all the detections suggested by each detector, however if
% there are overlaping detections from different detector, we are going to
% pick the detection with the highest score only.
for k = 1:Nclasses, 
    Score=Fx(:,:,k);
    % Look at local maximum of output score and output a set of detected object
    % bounding boxes.
    s(:,:,k) = double(Score>0);  
    s(:,:,k) = conv2(hamming(128),hamming(48),s(:,:,k),'same');  % Modified by Juan
    BW = imregionalmax(s(:,:,k));
    [y, x] = find(BW.*s(:,:,k));
    yt=[yt;y];
    xt=[xt;x];
    kt=[kt;repmat(k,length(x),1)];
end
 
x=xt;
y=yt;

D = distance([x y]',[x y]'); D = D + 1000*eye(size(D));

while min(D(:))<15  % This number is arbitrary. However represents the distance threshold of close pixels. It is clear that this number depends on the scale used.
    N = length(x);
    [i,j] = find(D==min(D(:)));
    
    %Lets check if points i and j belong to the same detector, or not. If yes we will take the average, and drop one point. If not, we will pick the detection with the highest score
    p1=i(1); p2=j(1);
    d=p2; % Drop p2
    if kt(p1)==kt(p2)   % The responses are from the same detector?
        x(p1) = round((x(p1) + x(p2))/2);
        y(p1) = round((y(p1) + y(p2))/2);
    else
         % If not, get the detection with the highest score between the overlapping
         % detections
         
         ind1 = sub2ind(size(s), y(p1), x(p1), kt(p1));
         ptScore1 = s(ind1);
         ind2 = sub2ind(size(s), y(p2), x(p2), kt(p2));
         ptScore2 = s(ind2);
         if ptScore1<ptScore2, d=p1; end  % If the second point is higher than the first, drop the first
         
    end
    x = x(setdiff(1:N,d));
    y = y(setdiff(1:N,d));
    kt = kt(setdiff(1:N,d));
    D = distance([x y]',[x y]'); D = D + 1000*eye(size(D));     
end

for k = 1:Nclasses, 
    [id val]=find(kt==k);
    nDetections{k} = length(x(id));
    if length(x(id))==0 & size(id,2)==0, id=zeros(0,1); end
    boundingBox{k} = repmat(data.averageBoundingBox{k}, [nDetections{k} 1]) + [x(id) x(id) y(id) y(id)];
    ind = sub2ind(size(s), y(id), x(id), kt(id));
    boxScores{k} = s(ind);
end