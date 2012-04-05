/******************************************************************************
*
*  NSSDC/CDF                  Quick Start Test Program (INTERNAL interface/C).
*
*  Version 1.11, 13-Jun-96, Hughes STX.
*
******************************************************************************/

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cdf.h"

#define EXIT_SUCCESS_   0
#define EXIT_FAILURE_   1

void QuitCDF PROTOARGs((char *where, CDFstatus status));

/******************************************************************************
* Main.
******************************************************************************/

int main () {
CDFid id;
CDFstatus status;
long varNum;
long dim[1] = {0}, counts[1] = {1}, intervals[1] = {1};
float aa;

/******************************************************************************
* Display title.
******************************************************************************/

/******************************************************************************
* Create variables.
******************************************************************************/
  status = CDFlib (OPEN_, CDF_, "david", &id,
                   NULL_);
  if (status < CDF_OK) QuitCDF ("5.0", status);
  status = CDFlib (GET_, zVAR_NUMBER_, "temp_lop", &varNum,
                   NULL_);
  if (status < CDF_OK) QuitCDF ("6.0", status);
  status = CDFlib (SELECT_, zVAR_, varNum, 
                            zVAR_RECNUMBER_, 0L,
                            zVAR_RECINTERVAL_, 1L,
                            zVAR_RECCOUNT_, 1L,
                            zVAR_DIMINDICES_, dim,
                            zVAR_DIMCOUNTS_, counts,
                            zVAR_DIMINTERVALS_, intervals,
                   GET_, zVAR_DATA_, &aa,
                   NULL_);
  if (status < CDF_OK) QuitCDF ("7.0", status); 
  printf("data= %8.2f %g %f %e %8.2e %9.2g\n",aa,aa,aa,aa,aa,aa);

  status = CDFlib (GET_, zVAR_NUMBER_, "tratio_lop", &varNum,
                   NULL_);
  if (status < CDF_OK) QuitCDF ("8.0", status);
  status = CDFlib (SELECT_, zVAR_, varNum,
                            zVAR_RECNUMBER_, 0L,
                            zVAR_RECINTERVAL_, 1L,
                            zVAR_RECCOUNT_, 1L,
                            zVAR_DIMINDICES_, dim,
                            zVAR_DIMCOUNTS_, counts,
                            zVAR_DIMINTERVALS_, intervals,
                   GET_, zVAR_DATA_, &aa,
                   NULL_);
  if (status < CDF_OK) QuitCDF ("9.0", status); 
  printf("data= %8.2f %g %f %e %8.2e %8.2g \n",aa,aa,aa,aa,aa,aa);

  status = CDFlib(CLOSE_, CDF_, 
                  NULL_);
  if (status < CDF_OK) QuitCDF ("a.0", status);
  
/******************************************************************************
* Successful completion.
******************************************************************************/

return EXIT_SUCCESS_;
}


/******************************************************************************
* QuitCDF.
******************************************************************************/

void QuitCDF (where, status)
char *where;
CDFstatus status;
{
  char text[CDF_STATUSTEXT_LEN+1];
  printf ("Aborting at %s...\n", where);
  if (status < CDF_OK) {
    CDFlib (SELECT_, CDF_STATUS_, status,
	    GET_, STATUS_TEXT_, text,
	    NULL_);
    printf ("%s\n", text);
  }
  CDFlib (CLOSE_, CDF_,
	  NULL_);
  printf ("...test aborted.\n");
  exit (EXIT_FAILURE_);
}
