clear all
parameters
testImageSize = [256 256];

% Load detector parameters:
load (dataFile);
load ('data/databaseStruct');
NweakClassifiers = length(data.detector);
%NweakClassifiers = [30 80 120 150]; % put a list if you want to compare performances with different number of weak learners.
%NweakClassifiers = [30 120]; % put a list if you want to compare
%performances with different number of weak learners.
ii=1; % counter of number of images total
% Define variables used for the precision-recall curve
scoreAxis = linspace(0, 300, 60); RET = []; REL = []; RETREL = [];
cmt=zeros(Nclasses-1,Nclasses-1,length(scoreAxis));  %Create the confusion matrix. rows and cols are the claases, and the
angulos=[0 90 270 180];
    %depth is the scoreAxis. 
% Create figures
sc = get(0, 'ScreenSize'); 
figSize = [.1*sc(3) .7*sc(4) .8*sc(3) .2*sc(4)];
Hfig = figure;
set (Hfig, 'position', figSize);
figSize = [.1*sc(3) .4*sc(4) .8*sc(3) .2*sc(4)];
Hfig2 = figure;
set (Hfig2, 'position', figSize);
plotstyle = {'rs-', 'go-', 'b^-', 'm*-'};

for obj = 1:(Nclasses-1),    
    [Dc, jc]  = LMquery(data.databaseStruct, 'object.name', objects.name{obj});

    trainedImages=data.image(find(data.class==obj));
    % remove images used to create the dictionary:
    testImages = setdiff(jc, [trainedImages; data.dictionary.imagendx']);
    NtestImages = length(testImages);
    
    % Loop on test images
    for i = 1:NtestImages
        % Read image and ground truth
        Img = LMimread(data.databaseStruct, testImages(i), HOMEIMAGES);
        annotation = data.databaseStruct(testImages(i)).annotation;
        ann = annotation;
        % Normalize image:
        [newannotation, newimg, crop, scaling, err, msg] = LMcookimage(annotation, Img, ...
            'objectname', objects.name{obj}, 'objectsize', normalizedObjectSize, 'objectlocation', 'original', 'maximagesize', testImageSize);

        for m = 1:length(NweakClassifiers)
            % Run derector at a single scale (you can loop on scales to get scale invariance):
            %[Score, boundingBox, boxScores] = singleScaleBoostedDetector(double(mean(newimg,3)), data, NweakClassifiers(m));

            %[Score, boundingBox, boxScores] = singleScaleJointDetector(double(mean(newimg,3)), data, T, NweakClassifiers(m));
            [Score, boundingBox, boxScores] = singleScaleJointDetector(double(mean(newimg,3)), data, T , NweakClassifiers(m));

            % Evaluate performace looking at precision-recall with different
            % thresholds:
            cfm{ii}=cmt; %zeros
            for n = 1:length(scoreAxis);
                % Now lets find the ROC plot 
                for kl=1: (Nclasses-1),
                        %boundedbox=[boundedbox;boundingBox{kl}(boxScores{kl}>scoreAxis(n),:)]; %adds up all the detections from all the classes together
                        boundedbox=boundingBox{kl}(boxScores{kl}>scoreAxis(n),:); %adds up all the detections from all the classes together
                        [RET{kl}(n,ii,m), REL{kl}(n,ii,m), RETREL{kl}(n,ii,m)] = LMprecisionRecall(boundedbox, newannotation, objects.name{obj}, [25 25]);       
                         % Create confusion matrix for image i
                         cfm{ii}(obj,kl,n)=RETREL{kl}(n,ii,m); %true positives detected      
                         if kl~=obj,  % If this detector is different than the class checked now
                             RETREL{kl}(n,ii,m)=0;  %Make the true hits be 0 since this class do not correspond to the one tested 
                             REL{kl}(n,ii,m)=0; %Make the targets be 0 since this class do not correspond to the one supplied 
                         end
                         
                end
            end
        end
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
                
       for kk=1:size(boxScores,2)
            sb = find(boxScores{kk}>scoreAxis(3)); % show bounding boxes with a high score.
           plotBoundingBoxLine(boundingBox{kk}(sb,:), 'r' , (fix(boxScores{kk}(sb,:)/40)+1),angulos(kk)); 
       end
     
        
        axis('equal'); axis('off'); axis('tight'); colormap(gray(256))
        title({'Detector output', sprintf('targets=%d, correct=%d, false alarms=%d',REL{obj}(3,ii,end), RETREL{obj}(3,ii,end), RET{obj}(3,ii,end)-RETREL{obj}(3,ii,end))})
        drawnow
        
        str = num2str(ii);
        filename=strcat('figures/test/',str,ann.filename);
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
      
        ii=ii+1;
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
