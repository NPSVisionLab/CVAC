function [m, seg] = zeroPad(m, N);
%
% Makes the matrix m to have size NxN by adding zeros

[sx,sy]=size(m);
m = [0*repmat(m(1,:), [fix((N(1)-sx)/2) 1]); m; 0*repmat(m(end,:), [fix((N(1)-sx)/2+.5) 1])];
[sx,sy]=size(m);
m = [0*repmat(m(:,1), [1 fix((N(2)-sy)/2)]) m 0*repmat(m(:,end), [1 fix((N(2)-sy)/2+.5)])];
