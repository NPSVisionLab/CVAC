function LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder)

% get number of images in the database
Nimages = length(database);

% % create directories for masks
mask_res = mkdir(base_folder, masks_folder);

obj_ctr = 0;

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
                        
            img_filename = database(i).annotation.filename;
            img_folder = database(i).annotation.folder;
            save_path = strcat(base_folder,'/',masks_folder,'/',img_folder,'_',img_filename,'_mask.jpg');
            res=exist(save_path); % the image exists?
                        
            for j = 1:Nobjects
                [mask, class] = LMobjectmask(database(i).annotation, HOMEIMAGES);
                mask=sum(mask,3);
                mask=(mask>=1);
                
                if res==0,
                   imwrite(mask, save_path, 'JPG', 'Quality', 100);
                end
                if res==2,
                    old_mask=imread(save_path);
                    maskcomp=or(mask,old_mask/255);
                    imwrite(maskcomp, save_path, 'JPG', 'Quality', 100);
                end
           end
    end
end
end



