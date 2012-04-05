function scaledimage = nihscale(image)
% nihscale(image)
% scales the pixel values of an image to make it like an nih image
% with 256 grey levels
%
% W. Dirks, 1998
%

% $Id: mb_nihscale.m,v 1.2 1999/02/17 14:19:56 boland Exp $

        s = image * 253/(max(max(image))-min(min(image)))+1;
        s = s/255;

scaledimage = s;

