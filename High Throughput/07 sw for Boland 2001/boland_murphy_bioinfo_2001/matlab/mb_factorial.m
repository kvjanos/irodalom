function factorial = mb_factorial(x)
% MB_FACTORIAL(X) calculates x! for scalars AND vectors
% MB_FACTORIAL(X) 
%
% 30 Nov 98 - M.V. Boland
%

% $Id: mb_factorial.m,v 1.2 1999/02/17 14:19:55 boland Exp $

if nargin ~= 1
	error('Please supply a single numeric argument.') ;
end

factorial=[] ;

for n=x
  factorial = [factorial prod(1:n)] ;
end
  
