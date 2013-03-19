clear all;

HOMEIMAGES = 'C:\POSTDOC\LabelMe\Images'; 
HOMEANNOTATIONS = 'C:\POSTDOC\LabelMe\Annotations'; 
LABELMEFOLDER = 'C:\\POSTDOC\\LabelMe\\Images';
%base_folder = 'C:/POSTDOC/Base_It/MultiBoost/Data';
base_folder = 'C:/Documents and Settings/nmlloyde/My Documents/MultiBoost/Data';
masks_folder = 'skull_masks';

LMdb = LMdatabase(HOMEANNOTATIONS);

query = 'skull+az0deg';
%query = 'marine+standing+az0deg,marine+walking+az0deg';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

query = 'skull+az90deg';
%query = 'marine+standing+az90deg,marine+walking+az90deg';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

query = 'skull+az180deg';
%query = 'marine+standing+az180deg,marine+walking+az180deg';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

query = 'skull+az270deg';
%query = 'marine+standing+az270deg,marine+walking+az270deg';
database = LMquery(LMdb, 'object.name', query);
LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
counts = LMcountobject(database, query);
LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);

%query = 'rifle+az0deg';
%database = LMquery(LMdb, 'object.name', query);
%database = LMquery(LMdb, 'object.name', query);
%LM2OpenCV2(database, HOMEIMAGES, LABELMEFOLDER,base_folder,query);
%counts = LMcountobject(database, query);
%LMCreateMasks(database, HOMEIMAGES, base_folder, masks_folder);