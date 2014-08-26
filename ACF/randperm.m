function p = randperm(n, k)
%RANDPERM Random permutation.
%   RANDPERM(n) is a random permutation of the integers from 1 to n.
%   For example, RANDPERM(6) might be [2 4 5 6 1 3].
%   
%   Note that RANDPERM calls RAND and therefore changes the state of the
%   random number generator that underlies RAND, RANDI, and RANDN.  Control
%   that shared generator using RNG.
%
%   See also RNG, PERMUTE.

%   Copyright 1984-2010 The MathWorks, Inc.
%   $Revision: 1.1.6.2.8.1 $  $Date: 2010/12/21 20:44:02 $

if(nargin < 2)
    k = n;
end

if(k > n)
    k = n;
end

[~,p] = sort(rand(1,n));
p = p(1:k);