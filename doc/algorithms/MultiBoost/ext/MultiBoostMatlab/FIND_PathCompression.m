function [parent] = FIND_PathCompression(temproot)

global ParentPointer;
ParentPointer(temproot);
if (ParentPointer(temproot)~=temproot)
    ParentPointer(temproot) = FIND_PathCompression(ParentPointer(temproot));
end
parent = ParentPointer(temproot);
