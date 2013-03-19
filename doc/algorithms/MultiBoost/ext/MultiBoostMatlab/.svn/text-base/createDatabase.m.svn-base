% You do not need to run this script for this demo. 
%
% You only need to run this script if you want to update the database. 
%
% 1) First, you can download the full LabelMe database from:
% http://people.csail.mit.edu/brussell/research/LabelMe/intro.html
% 2) Then, change the folders HOMEANNOTATIONS and HOMEIMAGES to point to the
% paths of the LabelMe database. 
% 3) Then, you can run this script to generate
% the LabelMe database struct that allows performing
% queries and extract objects and segmentation masks.
%
% This program only needs to be run once at the beggining. For the screen and car demo dataset, 
% this script has already been run, and the database struct is stored in:
% data/databaseStruct

clear all
close all

parameters

% Create database struct: This command is slow, so it is better to run it
% once at the beggining and then store the struct for use later.
D = LMdatabase(HOMEANNOTATIONS);
D = LMquery(D, 'object.deleted', '0'); % remove deleted objects

% Show some of the images with labels:
LMdbshowscenes(D(fix(linspace(1, length(D), 30))), HOMEIMAGES); 

% Save database struct for future use
%data.databaseStruct = D;
save ('data/databaseStruct', 'D');



