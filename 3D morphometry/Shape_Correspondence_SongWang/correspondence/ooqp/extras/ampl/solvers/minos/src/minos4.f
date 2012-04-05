      subroutine minos4( m, n, ne,
     $                   nncon, nnobj, nnjac, names,
     $                   ha, ka )

      implicit           double precision (a-h,o-z)
      character*8        names(5)
      integer*4          ha(ne)
      integer            ka(n+1)

      common    /m2len / mrows,mcols,melms
      common    /m3mps4/ name(2),mobj(2),mrhs(2),mrng(2),mbnd(2),minmax
      common    /m7len / fobj  ,fobj2 ,nnobjx,nnobj0
      common    /m8len / njac  ,nnconx,nncon0,nnjacx
      common    /m8al1 / penpar,rowtol,ncom,nden,nlag,nmajor,nminor

      character*5        f1
      data               f1 /'(2a4)'/
*     ------------------------------------------------------------------

*     Initialize timers.

      call m1time( 0,0 )

*     Load the Common variables with the nonlinear dimensions.

      mrows  = m
      mcols  = n
      melms  = ne
      nnconx = nncon
      nnobjx = nnobj
      nnjacx = nnjac

*     The Specs file has been read (or the options have been
*     otherwise defined).  Check that the options have sensible values.

      call m3dflt( 2 )

*     ------------------------------------------------------------------
*     Load names into the MINOS arrays.
*     ------------------------------------------------------------------
      read (names(1), f1) name
      read (names(2), f1) mobj
      read (names(3), f1) mrhs
      read (names(4), f1) mrng
      read (names(5), f1) mbnd

*     ------------------------------------------------------------------
*     Determine storage requirements using the
*     following Common variables:
*        (m2len )   mrows, mcols, melms
*        (m3len )   nscl  (determined by lscale)
*        (m5len )   maxr, maxs, nn
*        (m7len )   nnobj
*        (m8len )   njac, nncon, nnjac
*     All have to be known exactly before calling m2core( 4, ... ).
*     njac is the only one we don't yet know, if Jacobian = sparse.
*     If Jacobian = dense (nden = 1), m2core sets njac = nncon*nnjac.
*     ------------------------------------------------------------------

      njac = 0
      if (nncon .gt. 0  .and.  nden .eq. 2) then
         last = ka(nnjac+1) - 1
         if (nncon .eq. m) then
            njac = last
         else
            do 100 k = 1, last
               i     = ha(k)
               if (i .le. nncon) njac = njac + 1
  100       continue
         end if
      end if

        end
