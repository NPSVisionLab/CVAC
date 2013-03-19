%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Simple demo of Gentle Boost with stumps and 2D data
%
%
% Implementation of gentleBoost. The algorithm is described in:
% Friedman, J. H., Hastie, T. and Tibshirani, R. 
% "Additive Logistic Regression: a Statistical View of Boosting." (Aug. 1998) 

% atb, 2003
% torralba@ai.mit.edu

clear all

% Define plot style parameters
plotstyle.colors = {'gs', 'ro'};  % color for each class
plotstyle.range = [-50 50 -50 50]; % data range

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Create data: use the mouse to build the training dataset.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
% figure(1); clf
% axis(plotstyle.range); hold on
% axis('square')
% title('Left button = class +1, right button = class -1. Press any key to finish.')
% i  = 0; clear X Y
% while 1
%     [x,y,c] = ginput(1);
%     if ismember(c, [1 3])
%         i = i + 1;
%         X(1,i) = x;
%         X(2,i) = y;
%         Y(i) = (c==1) - (c==3); % class = {-1, +1}
%         plot(x, y, plotstyle.colors{(Y(i)+3)/2}, 'MarkerFaceColor', plotstyle.colors{(Y(i)+3)/2}(1), 'MarkerSize', 10);
%     else
%         break
%     end
% end
load DemoGentleBoostDato.mat
[Nfeatures, Nsamples] = size(X); 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Boosting parameters:
Nrounds = 100;

% Create test data: it is created on a grid  
[x1, x2] = meshgrid(linspace(plotstyle.range(1), plotstyle.range(2), 100)); 
[nrows,ncols] = size(x1);
xt = [x1(:) x2(:)]';

% Initialize output of strong classifiers on training and test set
Fx = 0; FxT = 0;

% Initialize to one all the weights for training samples
w  = ones(1, Nsamples);
figure
% Run boosting iterations and plot intermediate results
for m = 1:Nrounds
    disp(sprintf('Round %d', m))
    
    % Weak regression stump: It is defined by four parameters (a,b,k,th)
    %     f_m = a * (x_k > th) + b
    [k, th, a , b] = selectBestRegressionStump(X, Y, w);
    
    % Updating and computing classifier output on training set
    fm = (a * (X(k,:)>th) + b);
    Fx = Fx + fm;
    
    % Updating and computing classifier output on test set
    fmT = (a * (xt(k,:)>th) + b);
    FxT = FxT + fmT;
    
    % Visualization
    subplot(121)
    imagesc([plotstyle.range(1:2)], [plotstyle.range(3:4)], reshape(fmT, [nrows,ncols]))
    colormap(gray(256))
    hold on
    plotw(X(1,:), X(2,:), w, Y, plotstyle);
    axis('off')

    subplot(122)
    imagesc([plotstyle.range(1:2)], [plotstyle.range(3:4)], reshape(FxT>0, [nrows,ncols]))
    colormap(gray(256))
    hold on
    plotw(X(1,:), X(2,:), ones(size(w)), Y, plotstyle);
    title('Strong classifier')
    axis('off')

    % Reweight training samples
    w = w .* exp(-Y.*fm);
    
    %disp('Press a key to see next round')
    %pause
end




