%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Create dictionary of features
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% From each object class: get 10 images normalized in size, from each image
% filter the image with one filter (3 filters), from each output get 9
% fragments sampling location on a regular grid 3x3 => 10*3*9 = 270
% fragments for each object
%
% Store indices of images used for building the dictionary
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all

% Load parameters
parameters

% Set font size
%set(0,'defaultaxesfontsize',5);
%set(0,'defaulttextfontsize',5);

% Load database struct
load ('data/databaseStruct');
data.databaseStruct = D;
% Initialize patch counts
nd = 0;
    
for obt=1:(Nclasses-1),    
    averageBoundingBox = [];
    % Query for images that contain the target class:
    [Dc,jc]  = LMquery(D, 'object.name', objects.name{obt});
 
    % Maker sure that the images to create the dictionary are random and not
    % sequencies
    rind=randperm(length(jc));
    jc=jc(rind);
    ii=0;
    i=1;
    % Loop for extracting patches to build the dictionary
    while ii < sampleFromImages
        img = LMimread(D, jc(i), HOMEIMAGES);
        img = uint8(mean(double(img),3));  %Convert to grayscale 

        % Get tight crop of the centered object to extract patches:
        [newannotation, newimg, crop, scaling, err, msg] = LMcookimage(D(jc(i)).annotation, img, ...
            'objectname', objects.name{obt}, 'objectsize', normalizedObjectSize, 'objectlocation', 'centered', 'maximagesize', normalizedObjectSize + 60);

        [nrows, ncols] = size(newimg);
        if err == 0 | min(nrows,ncols)< min(normalizedObjectSize)

            % Get object polygon and object center coordinates
            [X,Y] = LMobjectpolygon(newannotation, objects.name{obt}); % get object polygon


            % Object center
            cx = round((min(X{1})+max(X{1}))/2);
            cy = round((min(Y{1})+max(Y{1}))/2);
            averageBoundingBox = [averageBoundingBox; min(X{1})-cx max(X{1})-cx min(Y{1})-cy max(Y{1})-cy];

            % segmentation mask
            [x,y] = meshgrid(1:ncols, 1:nrows);
            mask = logical(inpolygon(x, y, X{1}, Y{1}));
            % Sample points from edges within the object mask:
            edgemap = edge(newimg,'canny', [0.001 .01]);
            [yo, xo] = find(edgemap.*mask);
            no = randperm(length(xo(:))); no = no(1:patchesFromExample); % random sampling on edge points
            % The line above!!! We should do something else, we should find
            % maximum points on the edges (hoping that we have enough)
            xo = xo(no); yo = yo(no);

            % keep coordinates within image size:
            xo = max(xo, max(patchSize+1)/2+1);
            yo = max(yo, max(patchSize+1)/2+1);
            xo = min(xo, ncols - max(patchSize+1)/2);
            yo = min(yo, nrows - max(patchSize+1)/2);

            % Get patches from filtered image:
            out = convCrossConv(double(newimg), filters);

            % Crop patches: all filter outputs from each location
            for lp = 1:patchesFromExample
                Lx = 2*abs(xo(lp) - cx)+1;
                gx = zeros(1, Lx); gx((Lx+1)/2 - (xo(lp) - cx)) = 1;
                gx = conv(gx, locSigma);
                gx = gx/sum(gx); %Blurred delta function at teh relative offset

                Ly = 2*abs(yo(lp) - cy)+1;
                gy = zeros(1, Ly); gy((Ly+1)/2 - (yo(lp) - cy)) = 1;
                gy = conv(gy, locSigma);
                gy = gy/sum(gy); ; %Blurred delta function at teh relative offset

                for lf = 1:Nfilters
                    p = patchSize(fix(rand*length(patchSize))+1)-1; % random patch size
                    patch  = double(out(yo(lp)-p/2:yo(lp)+p/2, xo(lp)-p/2:xo(lp)+p/2, lf));
                    patch  = (patch - mean(patch(:))) / std(patch(:));

                    nd = nd+1; % counter of elements in the dictionary

                    % Store parameters in dictionary
                    dictionary.filter{nd} = filters{lf}; % Filter (feature)
                    dictionary.patch{nd}  = patch;       % Patch (template)
                    dictionary.location{nd}{1}  = gx;    % Location (part location)
                    dictionary.location{nd}{2}  = gy;
                    dictionary.imagendx(nd)  = jc(i);    % Index of image source   
                    figure(2)
                    %subplot(ceil(sqrt(Npatches)), ceil(sqrt(Npatches)), lf+(lp-1)*Nfilters)
                    subplot(ceil(Npatches/(Nfilters*2)), Nfilters*2, lf+(lp-1)*Nfilters)
                    imagesc(patch); 
                    axis('square'); colormap(gray(256)); drawnow
                end
            end


            str = num2str(i);
            filename=strcat('figures/dict/','fig1',num2str(obt),str,newannotation.scenedescription);
            saveas(gcf,filename, 'jpg');
            % Visualize what is done
            figure(1); clf
            subplot(1,2,1)
            LMplot(newannotation, newimg); axis('on');
            plot(cx, cy, 'rs', 'MarkerFaceColor','r')
            plot(xo, yo, 'gs', 'MarkerFaceColor','g')
            subplot(1,2,2)
            imshow(colorSegments(mask)); axis('on')
            drawnow
            
            
            str = num2str(i);
            filename=strcat('figures/dict/','fig2',num2str(obt),str,newannotation.scenedescription);
            saveas(gcf,filename, 'jpg');
            
            ii=ii+1;
         else
                disp(msg)
        end
        i=i+1;
    end

    averageBoundingBox = median(averageBoundingBox);
    data.averageBoundingBox{obt} = averageBoundingBox;
end
data.dictionary = dictionary;
% SAVE DICTIONARY
save (dataFile, 'data');




