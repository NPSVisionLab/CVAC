clear all;

HOMEIMAGES = 'C:\POSTDOC\LabelMe\Images'; 
HOMEANNOTATIONS = 'C:\POSTDOC\LabelMe\Annotations'; 
LABELMEFOLDER = 'C:\\POSTDOC\\LabelMe\\Images';
%base_folder = 'C:/POSTDOC/Base_It/MultiBoost/Data';
base_folder = 'C:/Documents and Settings/nmlloyde/My Documents/MultiBoost/Data';
masks_folder = 'hand_masks';

LMdb = LMdatabase(HOMEANNOTATIONS);

query = 'boxhand+victory';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

query = 'boxhand+sidepoint';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

query = 'boxhand+all_extended_open';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

query = 'boxhand+all_extended_closed';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

query = 'boxhand+Lback';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

query = 'boxhand+Lpalm';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);