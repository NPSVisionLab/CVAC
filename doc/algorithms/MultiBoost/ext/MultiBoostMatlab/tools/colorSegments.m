function img = colorSegments(cube)
% Transforms the segmentation mask into a color image. This  function is
% just intended as a visualization tool. 
% It might introduce artifacts if you try to use this function for segmenting objects.

[nrows, ncols, ncolor] = size(cube);

if ncolor>0
    map = hsv(ncolor);

    cube = reshape(cube, [nrows*ncols ncolor]);
    img = double(cube) * map;
    img = reshape(img, [nrows ncols 3]);

    img(img>1) = 1;

    img = uint8(255*img);
else
    % If it is an indexed map
end
