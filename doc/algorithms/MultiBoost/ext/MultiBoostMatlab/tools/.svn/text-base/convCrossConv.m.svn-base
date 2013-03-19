function out = convCrossConv(img, filters, patches, locations);
% out = convCrossConv(img, filters, patches, locations) 
%
%  z =  (|img * filter| ** patch) * location

[nrows, ncols, nc] = size(img);
Nfilters = length(filters);

% 1) Convolution (features)
out = zeros([nrows ncols Nfilters], 'single'); % This only works with Matlab 7. You have to remove 'single' to make it work on older versions.
for f = 1:Nfilters
    tmp = abs(conv2(double(img), filters{f}, 'same'));
    out(:,:,f) = conv2(tmp, [1 2 1; 2 4 2; 1 2 1]/16, 'same'); % Apply Gaussian to smooth the image
end
out = zeroBoundary(out, 1); % sets to zero one pixel at the image boundary

if nargin == 2; return; end

% 2) Normalized correlation (template matching) with each patch
for f = 1:Nfilters
    [n, m] = size(patches{f});
    %tmp = normxcorr2(patches{f}, double(out(:,:,f)));
    %tmp = tmp(fix(n/2)+1:end-ceil(n/2)+1, fix(m/2)+1:end-ceil(m/2)+1);
    tmp = normxcorr2_mex(patches{f}, double(out(:,:,f)), 'same');
    out(:,:,f) = zeroPad(single(tmp), [nrows ncols]);
end

if nargin == 3; return; end

% 3) Convolution (location): separable filters
% We exponentiate the correlation output to make the contrast of local maximum locations
% to be enhanced: The convolution approximates a local max.
for f = 1:Nfilters
    out(:,:,f) = conv2(locations{f}{2}, locations{f}{1}, out(:,:,f).^3, 'same');
end


