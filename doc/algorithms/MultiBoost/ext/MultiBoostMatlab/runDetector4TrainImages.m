clear all
parameters
nOS=normalizedObjectSize;
testImageSize = [256 256];

% Load detector parameters:
load (dataFile)
NweakClassifiers = length(data.detector);
NweakClassifiers = [120]; % put a list if you want to compare performances with different number of weak learners.
%NweakClassifiers = [30 120]; % put a list if you want to compare
%performances with different number of weak learners.
cc=1; % counter of number of images total
% Define variables used for the precision-recall curve
scoreAxis = linspace(0, 300, 60); RET = []; REL = []; RETREL = [];
cmt=zeros(Nclasses-1,Nclasses-1,length(scoreAxis));  %Create the confusion matrix. rows and cols are the claases, and the
    %depth is the scoreAxis. 
% Create figures
sc = get(0, 'ScreenSize'); 
figSize = [.1*sc(3) .7*sc(4) .8*sc(3) .2*sc(4)];
Hfig = figure;
set (Hfig, 'position', figSize);
figSize = [.1*sc(3) .4*sc(4) .8*sc(3) .2*sc(4)];
Hfig2 = figure;
set (Hfig2, 'position', figSize);
plotstyle = {'rs-', 'go-', 'b^-'};

for obj = 1:(Nclasses-1),    
    
    obj=2;  % DELETE THIS!!!!!!*******************************************************************************************************
    
    [Dc, jc]  = LMquery(data.databaseStruct, 'object.name', objects.name{obj});    
    trainedImages=unique(data.image(find(data.class==obj)));
    testImages = trainedImages;
    NtestImages = length(testImages);
    
    % Loop on test images
    i = 0; 
    ii = 0;
    
    testImages=511;   % DELETE THIS!!!!!!*******************************************************************************************************
    
    while ii < numTrainImages,
         i = i+1;
        % Read image and ground truth
        Img = LMimread(data.databaseStruct, testImages(i), HOMEIMAGES);
        annotation = data.databaseStruct(testImages(i)).annotation;
        ann = annotation;
        % Normalize image:
        [newannotation, newimg, crop, scaling, err, msg] = LMcookimage(annotation, Img, ...
            'objectname', objects.name{obj}, 'objectsize', normalizedObjectSize, 'objectlocation', 'original', 'maximagesize', trainingImageSize);
        
        [nrows, ncols] = size(newimg);
        
        if err == 0
                % Get object polygon
           [X,Y] = LMobjectpolygon(newannotation, objects.name{obj}); % get object polygon

                % Check that the object is inside the image. If it is cropped we
                % will not use it for training.
            if min(X{1})>-1 & min(Y{1})>-1 & max(X{1})<ncols+1 & max(Y{1})<nrows+1
                    imageUsed = 1;
                    ii = ii + imageUsed;
                    cc = cc + imageUsed;
                    [Score, boundingBox, boxScores] = singleScaleJointDetector(double(mean(newimg,3)), data, T);
                    
                    % DELETE THID BELOW
                    % ****************************************************
                    %fiat=data.features(1218,:);
                    %[Cx, Fx, Fn, Fstumps] = strongJointClassifier(fiat', data.detector , T);
                    
                    
                    %******************************************************
                    

                    % Evaluate performace looking at precision-recall with different
                    % thresholds:
                    m=1;
                    cfm{ii+1}=cmt; %zeros
                    for n = 1:length(scoreAxis);
                        % Now lets find the ROC plot 
                        for kl=1: (Nclasses-1),
                                %boundedbox=[boundedbox;boundingBox{kl}(boxScores{kl}>scoreAxis(n),:)]; %adds up all the detections from all the classes together
                                boundedbox=boundingBox{kl}(boxScores{kl}>scoreAxis(n),:); %adds up all the detections from all the classes together
                                [RET{kl}(n,cc,m), REL{kl}(n,cc,m), RETREL{kl}(n,cc,m)] = LMprecisionRecall(boundedbox, newannotation, objects.name{obj}, [nOS(1) nOS(2)]);       
                                 % Create confusion matrix for image i
                                 cfm{cc}(obj,kl,n)=RETREL{kl}(n,cc,m); %true positives detected      
                                 if kl~=obj,  % If this detector is different than the class checked now
                                     RETREL{kl}(n,cc,m)=0;  %Make the true hits be 0 since this class do not correspond to the one tested 
                                     REL{kl}(n,cc,m)=0; %Make the targets be 0 since this class do not correspond to the one supplied 
                                 end

                        end
                    end
                %end
                precision = 100 * squeeze(sum(RETREL{obj},2) ./ sum(RET{obj},2));
                recall    = 100 * squeeze(sum(RETREL{obj},2) ./ sum(REL{obj},2));

                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                % VISUALIZATION
                figure(Hfig); clf
                subplot(141)
                LMplot(newannotation, newimg); legend off
                title('Input image with ground truth')
                subplot(142)
                imagesc(Score); axis('equal'); axis('off'); axis('tight'); colormap(gray(256))
                title('Boosting margin')
                subplot(143)
                image(255*(Score>0)); axis('equal'); axis('off'); axis('tight'); colormap(gray(256))
                title('Thresholded output')
                subplot(144)
                image(newimg);

                sb1 = find(boxScores{1}>scoreAxis(1)); % show bounding boxes with a high score.
                plotBoundingBoxColor(boundingBox{1}(sb1,:),'r', fix(boxScores{1}(sb1)/40)+1);

                sb2 = find(boxScores{2}>scoreAxis(1)); % show bounding boxes with a high score.
                plotBoundingBoxColor(boundingBox{2}(sb2,:),'g', fix(boxScores{2}(sb2)/40)+1);

                sb3 = find(boxScores{3}>scoreAxis(1)); % show bounding boxes with a high score.
                plotBoundingBoxColor(boundingBox{3}(sb3,:),'b', fix(boxScores{3}(sb3)/40)+1);

                axis('equal'); axis('off'); axis('tight'); colormap(gray(256))
                title({'Detector output', sprintf('targets=%d, correct=%d, false alarms=%d',REL{obj}(1,cc,end), RETREL{obj}(1,cc,end), RET{obj}(1,cc,end)-RETREL{obj}(1,cc,end))})
                drawnow

                str = num2str(ii);
                filename=strcat('figures/test (trained)/',str,ann.filename);
                saveas(gcf,filename, 'jpg');

                figure(Hfig2); clf
                %subplot(144)
                for m = 1:length(NweakClassifiers)
                    h(m) = plot(recall(:,m), precision(:,m), plotstyle{m}, 'linewidth', 3); hold on
                end
                legend(h, num2str(NweakClassifiers(:)), 'location', 'best')
                xlabel('recall'); ylabel('precision')
                axis([0 100 0 100]); %axis('square'); 
                grid on
            end
        end
    end
end
  %Added by Juan
for kl=1: (Nclasses-1),
    data.RET{kl} = RET{kl};
    data.REL{kl} = REL{kl};
    data.RETREL{kl}=RETREL{kl};
end
data.cfm=cfm;
save (dataFile, 'data');
