function mask = mb_cropthresh(image, cropimage)
% MB_CROPTHRESH generates a cropped, thresholded, cleaned  image
% MB_CROPTHRESH(IMAGE, CROPIMAGE), where iamge is the IMAGE 
%    to be processed and CROPIMAGE is a binary mask defining
%    a region of interest.  Use CROPIMAGE=[] to process the 
%    entirity of IMAGE.  Thresholding is done AFTER applying
%    the region of interest.  The image is cleaned using the 
%    majority operation of bwmorph.
%
% 10 Jan 1999

% $Id: mb_cropthresh.m,v 1.4 1999/03/03 20:08:08 boland Exp $

%
% If the crop image exists, make all pixels outside the masked area
%   equal to 0.
%
if (~isempty(cropimage))
        image = roifilt2(0, image, ~cropimage) ;
end

%
% Use the thresholding technique from NIH image (REF?)
%
Iscaled = mb_nihscale(image) ;
Timage = mb_nihthreshold(Iscaled) ;
Ithresh = im2bw(Iscaled, Timage) ;

%
% The majority filter works well empirically to remove extraneous objects
%
mask = bwmorph(Ithresh, 'majority') ;




