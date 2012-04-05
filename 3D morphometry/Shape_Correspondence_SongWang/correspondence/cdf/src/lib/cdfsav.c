/******************************************************************************
*
*  NSSDC/CDF                                           CDF `save' operations.
*
*  Version 1.0, 31-Jan-06, Hughes STX.
*
*  Modification history:
*
*   V1.0  31-Jan-06, M Liu     Original version 
*
******************************************************************************/

#include "cdflib.h"

/******************************************************************************
* CDFsav.
******************************************************************************/

STATICforIDL CDFstatus CDFsav (Va, Cur)
struct VAstruct *Va;
struct CurStruct *Cur;
{
  CDFstatus pStatus = CDF_OK;
  switch (Va->item) {
    /**************************************************************************
    * CDF_
    *   Save a CDF for the dotCDF file.
    **************************************************************************/
    case CDF_: {
      struct CDFstruct *CDF; 
      /************************************************************************
      * Validate the current CDF.  Don't use `SelectCDF' here because we don't
      * want to return NO_MORE_ACCESS if that is the case.
      ************************************************************************/
      if (Cur->cdf == NULL)
	return NO_CDF_SELECTED;
      else
	CDF = Cur->cdf;
      /************************************************************************
      * If this is a CDF for which access was aborted, simply free the CDF's
      * memory.
      ************************************************************************/
      if (CDF->magic == ABORTEDid_MAGIC_NUMBER) {
	KillAbortedCDF (CDF, Cur);
	break;
      }
      /************************************************************************
      * If the CDF is open read/write, update the dotCDF file.
      ************************************************************************/
      if (CDF->status == READ_WRITE) {
	if (!sX(UpdateDotCDF(CDF),&pStatus)) {
	  AbortAccess (CDF, noUPDATE, noDELETE);
	  KillAbortedCDF (CDF, Cur);
	  return pStatus;
	}
      }
      /************************************************************************
      * If the CDF is compressed...
      ************************************************************************/
      if (CDF->uDotFp != NULL) {
	/**********************************************************************
	* If the CDF is open read/write...
	**********************************************************************/
	if (CDF->status == READ_WRITE) {
	  /********************************************************************
	  * Flush the uncompressed dotCDF file.
	  ********************************************************************/
	  if (!FLUSHv(CDF->uDotFp)) {
	    AbortAccess (CDF, noUPDATE, noDELETE);
	    KillAbortedCDF (CDF, Cur);
	    return CDF_WRITE_ERROR;
	  }
	}
      }
      else {
        /**********************************************************************
	* Not compressed.
	**********************************************************************/
      }
      break;
    }
    /**************************************************************************
    * rVAR_/zVAR_
    *    Save the current r/zVariable file.
    **************************************************************************/
    case rVAR_:
    case zVAR_: {
      Logical zOp = (Va->item == zVAR_);
      struct CDFstruct *CDF; struct VarStruct *Var;
      SelectCDF (Cur->cdf, CDF)
      if (BADzOP(CDF,!zOp)) return ILLEGAL_IN_zMODE;
      if (!CURRENTvarSELECTED(CDF,zOp)) return NO_VAR_SELECTED;

    if (zModeON(CDF))
      if (CDF->CURzVarNum < CDF->NrVars)
        Var = CDF->rVars[(int)CDF->CURzVarNum];
      else
        Var = CDF->zVars[(int)(CDF->CURzVarNum - CDF->NrVars)];
    else
      Var = BOO(zOp,CDF->zVars[(int)CDF->CURzVarNum],
                    CDF->rVars[(int)CDF->CURrVarNum]);

    if (CDF->singleFile) {
	sX (SINGLE_FILE_FORMAT, &pStatus);
	break;
      }
      if (Var != NULL) {
	if (Var->fp != NULL) {
          if (!FlushCache(Var->fp,Var->fp->cacheHead)) {
	    AbortAccess (CDF, UPDATE, noDELETE);
	    return VAR_SAVE_ERROR;
	  }
	  Var->fp = NULL;
	}
	else {
	  if (!sX(VAR_ALREADY_CLOSED,&pStatus)) return pStatus;
	}
      }
      else {
	if (!sX(VAR_ALREADY_CLOSED,&pStatus)) return pStatus;
      }
      break;
    }
    /**************************************************************************
    * Unknown item, must be the next function.
    **************************************************************************/
    default: {
      Va->fnc = Va->item;
      break;
    }
  }
  return pStatus;
}
