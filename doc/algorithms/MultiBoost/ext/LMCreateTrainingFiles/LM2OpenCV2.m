function LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER, base_folder,query)
% LM2OpenCV2 Outputs query in OpenCV Haar-detector format.
%   LM2OpenCV2(database, HOMEIMAGES, base_folder)
%   converts and stores the object images specified by the LabelMe database
%   struct to the OpenCV haar-detector format. base_folder specifies the
%   target directory for which positive training text file is 
%    stored for input to the OpenCV haar-detector training utility
%   (createsamples.exe and haartraining.exe).
%
%   To form positive training examples, a file called positives.txt is
%   created, which lists for each image in database, the filename and the
%   object bounding boxes 
%
%   The variable HOMEIMAGES specifies the physical location of the LabelMe
%   database used by the database index database.
%
%   To understand how to train an OpenCV haar-detector with the training
%   samples and files  generated by this script use example command line
%   calls are provided below.
%
%   Example (Pedestrian Detector):
%
%      (In MATLAB)
%
%      HOMEIMAGES = 'C:/LabelMe/Images';
%      HOMEANNOTATIONS = 'C:/LabelMe/Annotations';
%      base_folder = 'D:/dtd';
%      num_neg = 10000;
%      patch_size = [16 32]; % width=16, height=32
%
%      LMdb = LMdatabase(HOMEANNOTATIONS);
%      query = 'pedestrian,person,human,man,woman';
%      database = LMquery(LMdb, 'object.name', query);
%      LM2OpenCV(database, HOMEIMAGES, base_folder, num_neg, patch_size);
%      counts = LMcountobject(database, query);
%
%      (Command Prompt, Let counts=900)
%
%      createsamples -info D:\dtd\positive.txt -vec D:\dtd\positives.vec
%      -num 900 -w 16 -h 32
%
%      haartraining -data D:\dtd\peddetector\ -vec D:\dtd\positives.vec -bg
%      D:\dtd\negatives.txt -npos 900 -nneg 10000 -w 16 -h 32
%
%   In the above example, the call to createsamples generates the file
%   positives.vec used by the haartraining routine. The haartraining
%   routine then creates the OpenCV Haar pedestrian detector and saves it
%   in the director 'D:\dtd\peddetector'.
%

% get number of images in the database
Nimages = length(database);


% sample negatives evenly from each image
% nneg_per_image = ceil(num_neg/Nimages);
% if(nneg_per_image<1)
%     nneg_per_image = 1;
% end
% 
% % create directories positives and negatives
% pos_res = mkdir(base_folder, 'positives');
% neg_res = mkdir(base_folder, 'negatives');
% if ~(pos_res & neg_res)
%     error('LM2OpenCV error: Unable to create positives and/or negatives directories.');
% end

% create files positives.txt and negatives.txt
%pbdir = sprintf('%s/positives',base_folder);
%nbdir = sprintf('%s/negatives',base_folder);
pfilename = sprintf(strcat(base_folder,'/',query,'.txt'), base_folder);
%nfilename = sprintf('%s/negatives.txt', base_folder);
pos_file = fopen(pfilename, 'w+t');
%neg_file = fopen(nfilename, 'w+t');
if (pos_file<0)
    error('LM2OpenCV error: Unable to create files positive.txt and/or negatives.txt.');
end

% traverse images:
%   - copy over positive image files to the positives directory
%   - crop negative examples and store in negatives directory
%   - create files positives.txt and negatives.txt
%neg_ctr = 0;
obj_ctr = 0;
%pw = patch_size(1);
%ph = patch_size(2);
for i = 1:Nimages
    if isfield(database(i).annotation, 'object')
        Nobjects = length(database(i).annotation.object);
        try
            % load image
            img = LMimread(database, i, HOMEIMAGES); % Load image
            [nrows ncols c] = size(img);
            if (c==3) % convert to grayscale? (I'm not sure if this is required by OpenCV...)
                img = rgb2gray(img);
            end;
                        
            % crop and add positive examples from image i to positive image
            % directory
            bboxes = []; % store bounding boxes for negative patch generation
            
            img_filename = database(i).annotation.filename;
            img_folder = database(i).annotation.folder;
            
            fprintf(pos_file, strcat(LABELMEFOLDER,'\\',img_folder,'\\',img_filename,'\t %d'), Nobjects);
            
            for j = 1:Nobjects
                [X,Y] = getLMpolygon(database(i).annotation.object(j).polygon);
                
                % compute bounding box of object
                x = min(X)-2;
                y = min(Y)-2;
                w = max(X)+2 - x;
                h = max(Y)+2 - y;
                ctr_x = x+w/2;
                ctr_y = y+h/2;
                
                % round width and height to be a multiple of the patch size
                % width and height, and re-center bounding box
%                 sfactor = max(w/pw,h/ph);
%                 w = floor(sfactor*pw);
%                 h = floor(sfactor*ph);
%                 x = floor(ctr_x - w/2);
%                 y = floor(ctr_y - h/2);
                
                % make sure that patch fits inside image (otherwise skip
                % this positive)
                if (w>ncols || h>nrows)
                    continue;
                end
                
                % check boundaries
                if(x<1); x = 1; end;
                if(y<1); y = 1; end;
                if((x+w)>ncols); x = ncols-w; end;
                if((y+h)>nrows); y = nrows-h; end;
                
                % crop and save image in positive image directory
                %
                %cimg = img(y:(y+h-1),x:(x+w-1));
                %imwrite(cimg, img_filename, 'JPG', 'Quality', 100);
                
                % add entry into positives file:
                % OpenCV format is upper-left corner (x,y), width (w) and
                % height (h)                
                fprintf(pos_file, ' %d %d %d %d', x,y,w,h);
                
                %'positives\\pos%06d.jpg\t1\t0 0 %d %d\n', obj_ctr, w, h);
                
                % use bboxes below...
                bboxes = [bboxes; x y w h];
                
                % next object
                obj_ctr = obj_ctr + 1;
            end
             fprintf(pos_file, '\n'); 
        end
    end
end

% done.
fclose(pos_file);


