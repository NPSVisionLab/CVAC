function [class_idx,idxt] = SeqPartition(boxes,scores,k)
boiies=boxes;

% Convert the bounding box to [x1 y1 with height] instead of [x1 x2 y1 y2]
boxes(:,2)=boxes(:,2)-boxes(:,1); 
boxes(:,4)=boxes(:,4)-boxes(:,3); 
temp=boxes(:,3);
boxes(:,3)=boxes(:,2);   
boxes(:,2)=temp;         

n=size(boxes,1);

% We use the Disjoint sets data structures to detect cycle while adding new
% edges. Union by Rank with path compression is implemented here.

% Assign parent pointers to each vertex. Initially each vertex points to 
% itself. Now we have a conceptual forest of n trees representing n disjoint 
% sets 
global ParentPointer ;
ParentPointer = 0;
ParentPointer(1:n) = 1:n;

% Assign a rank to each vertex (root of each tree). Initially all vertices 
% have the rank zero.
TreeRank = 0;
TreeRank(1:n) = 0;
class_idx = 0;

% Visit each edge in the sorted edges array
% If the two end vertices of the edge are in different sets (no cycle), add
% the edge to the set of edges in minimum spanning tree
%MSTreeEdges = 0;
%MSTreeEdgesCounter = 0; i = 1;

% The main O(N^2) pass. Merge connected components.
node=0; node2=0; root=0;
for i=1:n,
    node=i;
    %find root
    root = FIND_PathCompression(node);
    for j=1:n,
       node2=j;
       if ((node2 ~= node)& (is_equal(boxes(node,:),boxes(node2,:),k))),
            %find root
           root2= FIND_PathCompression(node2);
           %unite both trees placing the tree with the smaller rank under
           %the one with the higher rank
           if (root2 ~= root)
               if (TreeRank(root)>TreeRank(root2)),
                   ParentPointer(root2)=root;
               else
                   ParentPointer(root)=root2;
                   TreeRank(root2) = TreeRank(root2) + (TreeRank(root) == TreeRank(root2));
                   root = root2;
               end    
               
               node2 = FIND_PathCompression(node2);
               %temp->parent = root; ??
               node2=node;
               node2 = FIND_PathCompression(node2);
           end
           
       end
    end
end

idxt=zeros(n,1);
% Final O(N) pass (Enumerate classes)
for i=1:n,
   idx=-1;
   node=i;
   node = FIND_PathCompression(node);
   if TreeRank(node)>=0,
       TreeRank(node)=-1*class_idx-1;
       class_idx=class_idx+1;
   end    
    idx = -1*TreeRank(node)-1;
    idxt(i)=idx;
end

