/******************************************************************************
*
*  NSSDC/CDF                                        Read from internal record.
*
*  Version 1.4a, 28-Feb-97, Hughes STX.
*
*  Modification history:
*
*   V1.0  30-Nov-94, J Love     Original version.
*   V1.1  30-Jan-95, J Love	`Read32s' now checks count.
*   V1.1a 24-Feb-95, J Love	Solaris 2.3 IDL i/f.
*   V1.2  26-May-95, J Love	CDF V2.4 compatibility mode.  What?
*   V1.3  14-Jun-95, J Love	Recursion!
*   V1.3a  4-Aug-95, J Love	More efficient `Read32' and `Read32s'.
*				CDFexport-related changes.
*   V1.4   3-Apr-96, J Love	CDF V2.6.
*   V1.4a 28-Feb-97, J Love	Windows NT for MS Visual C/C++ on an IBM PC.
*
******************************************************************************/

#include "cdflib.h"

/******************************************************************************
* Local macro definitions.
******************************************************************************/

#define CRE CDF_READ_ERROR

/******************************************************************************
* Read32.
******************************************************************************/

VISIBLE_PREFIX Logical Read32 (fp, value)
vFILE *fp;
Int32 *value;
{
#if defined(NETWORKbyteORDERcpu)
  if (!READv(value,(size_t)4,(size_t)1,fp)) return FALSE;
#else
  Int32 temp;
  if (!READv(&temp,(size_t)4,(size_t)1,fp)) return FALSE;
  REVERSE4bIO (&temp, value)
#endif
  return TRUE;
}

/******************************************************************************
* Read32s.
******************************************************************************/

STATICforIDL Logical Read32s (fp, buffer, count)
vFILE *fp;
Int32 *buffer;
int count;
{
#if defined(NETWORKbyteORDERcpu)
  if (count < 1) return TRUE;
  if (!READv(buffer,(size_t)4,(size_t)count,fp)) return FALSE;
#else
#define MAX_READ32s CDF_MAX_DIMS	/* This must be the maximum of
					   CDF_MAX_DIMS and MAX_VXR_ENTRIES
					   (and for any other uses of
					   `Read32s'). */
  int i; Int32 temp[MAX_READ32s];
  if (count < 1) return TRUE;
  if (!READv(temp,(size_t)4,(size_t)count,fp)) return FALSE;
  for (i = 0; i < count; i++) {
     REVERSE4bIO (&temp[i], &buffer[i])
  }
#endif
  return TRUE;
}

/******************************************************************************
* ReadIrSize.
*   The size is always in the first 4-byte field.
******************************************************************************/

STATICforIDL CDFstatus ReadIrSize (fp, offset, irSize)
vFILE *fp;
Int32 offset;
Int32 *irSize;
{
  if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
  if (!Read32(fp,irSize)) return CRE;
  return CDF_OK;
}  

/******************************************************************************
* ReadIrType.
*   The type is always in the second 4-byte field.
******************************************************************************/

VISIBLE_PREFIX CDFstatus ReadIrType (fp, offset, irType)
vFILE *fp;
Int32 offset;
Int32 *irType;
{
  long irTypeOffset = offset + sizeof(Int32);
  if (!SEEKv(fp,irTypeOffset,vSEEK_SET)) return CRE;
  if (!Read32(fp,irType)) return CRE;
  return CDF_OK;
}  

/******************************************************************************
* ReadCDR.
*   Note that the length of the CDF copyright was decreased in CDF V2.5 (there
* were way too many characters allowed for).  When reading the copyright, only
* CDF_COPYRIGHT_LEN characters will be read.  This will be less than the
* actual number in CDFs prior to CDF V2.4 but is enough to include all of the
* characters that were used.  (The value of CDF_COPYRIGHT_LEN was decreased
* from CDF V2.4 to CDF V2.5.)
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadCDR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadCDR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case CDR_NULL:
	 va_end (ap);
	 return pStatus;
       case CDR_RECORD: {
	 struct CDRstruct *CDR = va_arg (ap, struct CDRstruct *);
	 void *copyRight = va_arg (ap, char *);
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(CDR->RecordSize))) return CRE;
	 if (!Read32(fp,&(CDR->RecordType))) return CRE;
	 if (!Read32(fp,&(CDR->GDRoffset))) return CRE;
	 if (!Read32(fp,&(CDR->Version))) return CRE;
	 if (!Read32(fp,&(CDR->Release))) return CRE;
	 if (!Read32(fp,&(CDR->Encoding))) return CRE;
	 if (!Read32(fp,&(CDR->Flags))) return CRE;
	 if (!Read32(fp,&(CDR->rfuA))) return CRE;
	 if (!Read32(fp,&(CDR->rfuB))) return CRE;
	 if (!Read32(fp,&(CDR->Increment))) return CRE;
	 if (!Read32(fp,&(CDR->rfuD))) return CRE;
	 if (!Read32(fp,&(CDR->rfuE))) return CRE;
	 if (copyRight != NULL) {
	   if (!READv(copyRight,CDF_COPYRIGHT_LEN,1,fp)) return CRE;
	   NulPad (copyRight, CDF_COPYRIGHT_LEN);
	 }
	 break;
       }
       case CDR_COPYRIGHT: {
	 void *copyRight = va_arg (ap, char *);
	 long tOffset = offset + CDR_COPYRIGHT_OFFSET;
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!READv(copyRight,CDF_COPYRIGHT_LEN,1,fp)) return CRE;
	 NulPad (copyRight, CDF_COPYRIGHT_LEN);
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *);
	 long tOffset = offset;
	 switch (field) {
	   case CDR_RECORDSIZE: tOffset += CDR_RECORDSIZE_OFFSET; break;
	   case CDR_RECORDTYPE: tOffset += CDR_RECORDTYPE_OFFSET; break;
	   case CDR_GDROFFSET: tOffset += CDR_GDROFFSET_OFFSET; break;
	   case CDR_VERSION: tOffset += CDR_VERSION_OFFSET; break;
	   case CDR_RELEASE: tOffset += CDR_RELEASE_OFFSET; break;
	   case CDR_ENCODING: tOffset += CDR_ENCODING_OFFSET; break;
	   case CDR_FLAGS: tOffset += CDR_FLAGS_OFFSET; break;
	   case CDR_INCREMENT: tOffset += CDR_INCREMENT_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadGDR.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadGDR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadGDR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case GDR_NULL:
	 va_end (ap);
	 return pStatus;
       case GDR_RECORD: {
	 struct GDRstruct *GDR = va_arg (ap, struct GDRstruct *);
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(GDR->RecordSize))) return CRE;
	 if (!Read32(fp,&(GDR->RecordType))) return CRE;
	 if (!Read32(fp,&(GDR->rVDRhead))) return CRE;
	 if (!Read32(fp,&(GDR->zVDRhead))) return CRE;
	 if (!Read32(fp,&(GDR->ADRhead))) return CRE;
	 if (!Read32(fp,&(GDR->eof))) return CRE;
	 if (!Read32(fp,&(GDR->NrVars))) return CRE;
	 if (!Read32(fp,&(GDR->NumAttr))) return CRE;
	 if (!Read32(fp,&(GDR->rMaxRec))) return CRE;
	 if (!Read32(fp,&(GDR->rNumDims))) return CRE;
	 if (!Read32(fp,&(GDR->NzVars))) return CRE;
	 if (!Read32(fp,&(GDR->UIRhead))) return CRE;
	 if (!Read32(fp,&(GDR->rfuC))) return CRE;
	 if (!Read32(fp,&(GDR->rfuD))) return CRE;
	 if (!Read32(fp,&(GDR->rfuE))) return CRE;
	 if (!Read32s(fp,GDR->rDimSizes,(int)GDR->rNumDims)) return CRE;
	 break;
       }
       case GDR_rDIMSIZES: {
	 Int32 *rDimSizes = va_arg (ap, Int32 *); Int32 rNumDims; long tOffset;
	 if (!sX(ReadGDR(fp,offset,
			 GDR_rNUMDIMS,&rNumDims,
			 GDR_NULL),&pStatus)) return pStatus;
	 tOffset = offset + GDR_rDIMSIZES_OFFSET;
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32s(fp,rDimSizes,(int)rNumDims)) return CRE;
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case GDR_RECORDSIZE: tOffset += GDR_RECORDSIZE_OFFSET; break;
	   case GDR_RECORDTYPE: tOffset += GDR_RECORDTYPE_OFFSET; break;
	   case GDR_rVDRHEAD: tOffset += GDR_rVDRHEAD_OFFSET; break;
	   case GDR_zVDRHEAD: tOffset += GDR_zVDRHEAD_OFFSET; break;
	   case GDR_ADRHEAD: tOffset += GDR_ADRHEAD_OFFSET; break;
	   case GDR_EOF: tOffset += GDR_EOF_OFFSET; break;
	   case GDR_NrVARS: tOffset += GDR_NrVARS_OFFSET; break;
	   case GDR_NUMATTR: tOffset += GDR_NUMATTR_OFFSET; break;
	   case GDR_rMAXREC: tOffset += GDR_rMAXREC_OFFSET; break;
	   case GDR_rNUMDIMS: tOffset += GDR_rNUMDIMS_OFFSET; break;
	   case GDR_NzVARS: tOffset += GDR_NzVARS_OFFSET; break;
	   case GDR_UIRHEAD: tOffset += GDR_UIRHEAD_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadADR.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadADR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadADR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case ADR_NULL:
	 va_end (ap);
	 return pStatus;
       case ADR_RECORD: {
	 struct ADRstruct *ADR = va_arg (ap, struct ADRstruct *);
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(ADR->RecordSize))) return CRE;
	 if (!Read32(fp,&(ADR->RecordType))) return CRE;
	 if (!Read32(fp,&(ADR->ADRnext))) return CRE;
	 if (!Read32(fp,&(ADR->AgrEDRhead))) return CRE;
	 if (!Read32(fp,&(ADR->Scope))) return CRE;
	 if (!Read32(fp,&(ADR->Num))) return CRE;
	 if (!Read32(fp,&(ADR->NgrEntries))) return CRE;
	 if (!Read32(fp,&(ADR->MAXgrEntry))) return CRE;
	 if (!Read32(fp,&(ADR->rfuA))) return CRE;
	 if (!Read32(fp,&(ADR->AzEDRhead))) return CRE;
	 if (!Read32(fp,&(ADR->NzEntries))) return CRE;
	 if (!Read32(fp,&(ADR->MAXzEntry))) return CRE;
	 if (!Read32(fp,&(ADR->rfuE))) return CRE;
	 if (!READv(ADR->Name,CDF_ATTR_NAME_LEN,1,fp)) return CRE;
	 NulPad (ADR->Name, CDF_ATTR_NAME_LEN);
	 break;
       }
       case ADR_NAME: {
	 char *aName = va_arg (ap, char *);
	 long tOffset = offset + ADR_NAME_OFFSET;
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!READv(aName,CDF_ATTR_NAME_LEN,1,fp)) return CRE;
	 NulPad (aName, CDF_ATTR_NAME_LEN);
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case ADR_RECORDSIZE: tOffset += ADR_RECORDSIZE_OFFSET; break;
	   case ADR_RECORDTYPE: tOffset += ADR_RECORDTYPE_OFFSET; break;
	   case ADR_ADRNEXT: tOffset += ADR_ADRNEXT_OFFSET; break;
	   case ADR_AgrEDRHEAD: tOffset += ADR_AgrEDRHEAD_OFFSET; break;
	   case ADR_SCOPE: tOffset += ADR_SCOPE_OFFSET; break;
	   case ADR_NUM: tOffset += ADR_NUM_OFFSET; break;
	   case ADR_NgrENTRIES: tOffset += ADR_NgrENTRIES_OFFSET; break;
	   case ADR_MAXgrENTRY: tOffset += ADR_MAXgrENTRY_OFFSET; break;
	   case ADR_AzEDRHEAD: tOffset += ADR_AzEDRHEAD_OFFSET; break;
	   case ADR_NzENTRIES: tOffset += ADR_NzENTRIES_OFFSET; break;
	   case ADR_MAXzENTRY: tOffset += ADR_MAXzENTRY_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadAEDR/AzEDR.
*   If the entry value is being read, it is passed back in the encoding of the
* CDF (no decoding is performed).  The caller must decode the value (if that
* is necessary).
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadAEDR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadAEDR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case AEDR_NULL:
	 va_end (ap);
	 return pStatus;
       case AEDR_RECORD: {
	 struct AEDRstruct *AEDR = va_arg (ap, struct AEDRstruct *);
	 void *value = va_arg (ap, void *); size_t nBytes;
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(AEDR->RecordSize))) return CRE;
	 if (!Read32(fp,&(AEDR->RecordType))) return CRE;
	 if (!Read32(fp,&(AEDR->AEDRnext))) return CRE;
	 if (!Read32(fp,&(AEDR->AttrNum))) return CRE;
	 if (!Read32(fp,&(AEDR->DataType))) return CRE;
	 if (!Read32(fp,&(AEDR->Num))) return CRE;
	 if (!Read32(fp,&(AEDR->NumElems))) return CRE;
	 if (!Read32(fp,&(AEDR->rfuA))) return CRE;
	 if (!Read32(fp,&(AEDR->rfuB))) return CRE;
	 if (!Read32(fp,&(AEDR->rfuC))) return CRE;
	 if (!Read32(fp,&(AEDR->rfuD))) return CRE;
	 if (!Read32(fp,&(AEDR->rfuE))) return CRE;
	 if (value != NULL) {
	   nBytes = (size_t) (CDFelemSize(AEDR->DataType) * AEDR->NumElems);
	   if (!READv(value,nBytes,1,fp)) return CRE;
	 }
	 break;
       }
       case AEDR_VALUE: {
	 void *value = va_arg (ap, void *);
	 size_t nBytes; Int32 dataType, numElems; long tOffset;
	 if (!sX(ReadAEDR(fp,offset,
			  AEDR_DATATYPE,&dataType,
			  AEDR_NUMELEMS,&numElems,
			  AEDR_NULL),&pStatus)) return pStatus;
	 tOffset = offset + AEDR_VALUE_OFFSET;
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 nBytes = (size_t) (CDFelemSize(dataType) * numElems);
	 if (!READv(value,nBytes,1,fp)) return CRE;
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case AEDR_RECORDSIZE: tOffset += AEDR_RECORDSIZE_OFFSET; break;
	   case AEDR_RECORDTYPE: tOffset += AEDR_RECORDTYPE_OFFSET; break;
	   case AEDR_AEDRNEXT: tOffset += AEDR_AEDRNEXT_OFFSET; break;
	   case AEDR_ATTRNUM: tOffset += AEDR_ATTRNUM_OFFSET; break;
	   case AEDR_DATATYPE: tOffset += AEDR_DATATYPE_OFFSET; break;
	   case AEDR_NUM: tOffset += AEDR_NUM_OFFSET; break;
	   case AEDR_NUMELEMS: tOffset += AEDR_NUMELEMS_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadVDR/zVDR.
*   If the pad value is being read, it is passed back in the encoding of the
* CDF (no decoding is performed).  The caller must decode the value (if that
* is necessary).
*   If this CDF contains wasted space in its VDRs, note that the offset for
* those fields after the wasted space is adjusted.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadVDR (struct CDFstruct *CDF, vFILE *fp,
				Int32 offset, Logical zVar, ...)
#else
STATICforIDL CDFstatus ReadVDR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, zVar);
#else
  struct CDFstruct *CDF; vFILE *fp; Int32 offset; Logical zVar;
  VA_START (ap);
  CDF = va_arg (ap, struct CDFstruct *);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
  zVar = va_arg (ap, Logical);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case VDR_NULL:
	 va_end (ap);
	 return pStatus;
       case VDR_RECORD: {
	 struct VDRstruct *VDR = va_arg (ap, struct VDRstruct *);
	 void *padValue = va_arg (ap, void *); Int32 nDims;
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(VDR->RecordSize))) return CRE;
	 if (!Read32(fp,&(VDR->RecordType))) return CRE;
	 if (!Read32(fp,&(VDR->VDRnext))) return CRE;
	 if (!Read32(fp,&(VDR->DataType))) return CRE;
	 if (!Read32(fp,&(VDR->MaxRec))) return CRE;
	 if (!Read32(fp,&(VDR->VXRhead))) return CRE;
	 if (!Read32(fp,&(VDR->VXRtail))) return CRE;
	 if (!Read32(fp,&(VDR->Flags))) return CRE;
	 if (!Read32(fp,&(VDR->sRecords))) return CRE;
	 if (!Read32(fp,&(VDR->rfuB))) return CRE;
	 if (!Read32(fp,&(VDR->rfuC))) return CRE;
	 if (!Read32(fp,&(VDR->rfuF))) return CRE;
	 if (CDF->wastedSpace) {
	   if (!SEEKv(fp,(long)VDR_WASTED_SIZE,vSEEK_CUR)) return CRE;
	 }
	 if (!Read32(fp,&(VDR->NumElems))) return CRE;
	 if (!Read32(fp,&(VDR->Num))) return CRE;
	 if (!Read32(fp,&(VDR->CPRorSPRoffset))) return CRE;
	 if (!Read32(fp,&(VDR->blockingFactor))) return CRE;
	 if (!READv(VDR->Name,CDF_VAR_NAME_LEN,1,fp)) return CRE;
	 NulPad (VDR->Name, CDF_VAR_NAME_LEN);
	 if (zVar) {
	   if (!Read32(fp,&(VDR->zNumDims))) return CRE;
	   if (!Read32s(fp,VDR->zDimSizes,(int)VDR->zNumDims)) return CRE;
	 }
	 if (zVar)
	   nDims = VDR->zNumDims;
	 else {
	   long tOffset = V_tell (fp); Int32 GDRoffset;
	   if (!sX(ReadCDR(fp,V2_CDR_OFFSET,
			   CDR_GDROFFSET,&GDRoffset,
			   CDR_NULL),&pStatus)) return pStatus;
	   if (!sX(ReadGDR(fp,GDRoffset,
			   GDR_rNUMDIMS,&nDims,
			   GDR_NULL),&pStatus)) return pStatus;
	   if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 }
	 if (!Read32s(fp,VDR->DimVarys,(int)nDims)) return CRE;
	 if (PADvalueBITset(VDR->Flags) && padValue != NULL) {
	   size_t nBytes = (size_t) (CDFelemSize(VDR->DataType)*VDR->NumElems);
	   if (!READv(padValue,nBytes,1,fp)) return CRE;
	 }
	 break;
       }
       case VDR_NAME: {
	 char *vName = va_arg (ap, char *);
	 long tOffset = offset + VDR_NAME_OFFSET;
	 if (CDF->wastedSpace) tOffset += VDR_WASTED_SIZE;
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!READv(vName,CDF_VAR_NAME_LEN,1,fp)) return CRE;
	 NulPad (vName, CDF_VAR_NAME_LEN);
	 break;
       }
       case VDR_zNUMDIMS: {
	 Int32 *numDims = va_arg (ap, Int32 *);
	 long tOffset = offset + zVDR_zNUMDIMS_OFFSET +
			BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0);
	 if (!zVar) return CDF_INTERNAL_ERROR;
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,numDims)) return CRE;
	 break;
       }
       case VDR_zDIMSIZES: {
	 Int32 *zDimSizes = va_arg (ap, Int32 *);
	 Int32 zNumDims; long tOffset;
	 if (!zVar) return CDF_INTERNAL_ERROR;
	 if (!sX(ReadVDR(CDF,fp,offset,zVar,
			 VDR_zNUMDIMS,&zNumDims,
			 VDR_NULL),&pStatus)) return pStatus;
	 tOffset = offset + zVDR_zDIMSIZES_OFFSET +
		   BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0);
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32s(fp,zDimSizes,(int)zNumDims)) return CRE;
	 break;
       }
       case VDR_DIMVARYS: {
	 Int32 *dimVarys = va_arg (ap, Int32 *);
	 Int32 nDims; long tOffset;
	 if (zVar) {
	   if (!sX(ReadVDR(CDF,fp,offset,zVar,
			   VDR_zNUMDIMS,&nDims,
			   VDR_NULL),&pStatus)) return pStatus;
	   tOffset = offset + zVDR_DIMVARYS_OFFSETb + (nDims*sizeof(Int32)) +
		     BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0);
	 }
	 else {
	   Int32 GDRoffset;
	   if (!sX(ReadCDR(fp,V2_CDR_OFFSET,
			   CDR_GDROFFSET,&GDRoffset,
			   CDR_NULL),&pStatus)) return pStatus;
	   if (!sX(ReadGDR(fp,GDRoffset,
			   GDR_rNUMDIMS,&nDims,
			   GDR_NULL),&pStatus)) return pStatus;
	   tOffset = offset + rVDR_DIMVARYS_OFFSET +
		     BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0);
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32s(fp,dimVarys,(int)nDims)) return CRE;
	 break;
       }
       case VDR_PADVALUE: {
	 void *padValue = va_arg (ap, void *);
	 Int32 dataType, numElems; size_t nBytes; long tOffset;
	 if (!sX(ReadVDR(CDF,fp,offset,zVar,
			 VDR_DATATYPE,&dataType,
			 VDR_NUMELEMS,&numElems,
			 VDR_NULL),&pStatus)) return pStatus;
	 if (zVar) {
	   Int32 zNumDims;
	   if (!sX(ReadVDR(CDF,fp,offset,zVar,
			   VDR_zNUMDIMS,&zNumDims,
			   VDR_NULL),&pStatus)) return pStatus;
	   tOffset = offset + zVDR_PADVALUE_OFFSETb +
		     (zNumDims*sizeof(Int32)) +
		     (zNumDims*sizeof(Int32)) +
		     BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0);
	 }
	 else {
	   Int32 rNumDims, GDRoffset;
	   if (!sX(ReadCDR(fp,V2_CDR_OFFSET,
			   CDR_GDROFFSET,&GDRoffset,
			   CDR_NULL),&pStatus)) return pStatus;
	   if (!sX(ReadGDR(fp,GDRoffset,
			   GDR_rNUMDIMS,&rNumDims,
			   GDR_NULL),&pStatus)) return pStatus;
	   tOffset = offset + rVDR_PADVALUE_OFFSETb +
		     (rNumDims*sizeof(Int32)) +
		     BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0);
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 nBytes = (size_t) (CDFelemSize(dataType) * numElems);
	 if (!READv(padValue,nBytes,1,fp)) return CRE;
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case VDR_RECORDSIZE: tOffset += VDR_RECORDSIZE_OFFSET; break;
	   case VDR_RECORDTYPE: tOffset += VDR_RECORDTYPE_OFFSET; break;
	   case VDR_VDRNEXT: tOffset += VDR_VDRNEXT_OFFSET; break;
	   case VDR_DATATYPE: tOffset += VDR_DATATYPE_OFFSET; break;
	   case VDR_MAXREC: tOffset += VDR_MAXREC_OFFSET; break;
	   case VDR_VXRHEAD: tOffset += VDR_VXRHEAD_OFFSET; break;
	   case VDR_VXRTAIL: tOffset += VDR_VXRTAIL_OFFSET; break;
	   case VDR_FLAGS: tOffset += VDR_FLAGS_OFFSET; break;
	   case VDR_sRECORDS: tOffset += VDR_sRECORDS_OFFSET; break;
	   case VDR_NUMELEMS:
	     tOffset += (VDR_NUMELEMS_OFFSET +
			 BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0));
	     break;
	   case VDR_NUM:
	     tOffset += (VDR_NUM_OFFSET +
			 BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0));
	     break;
	   case VDR_CPRorSPR:
	     tOffset += (VDR_CPRorSPR_OFFSET +
			 BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0));
	     break;
	   case VDR_BLOCKING:
	     tOffset += (VDR_BLOCKING_OFFSET +
			 BOO(CDF->wastedSpace,VDR_WASTED_SIZE,0));
	     break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadVXR.
******************************************************************************/

#if defined(STDARG)
VISIBLE_PREFIX CDFstatus ReadVXR (vFILE *fp, Int32 offset, ...)
#else
VISIBLE_PREFIX CDFstatus ReadVXR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case VXR_NULL:
	 va_end (ap);
	 return pStatus;
       case VXR_RECORD: {
	 struct VXRstruct *VXR = va_arg (ap, struct VXRstruct *);
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(VXR->RecordSize))) return CRE;
	 if (!Read32(fp,&(VXR->RecordType))) return CRE;
	 if (!Read32(fp,&(VXR->VXRnext))) return CRE;
	 if (!Read32(fp,&(VXR->Nentries))) return CRE;
	 if (!Read32(fp,&(VXR->NusedEntries))) return CRE;
	 if (!Read32s(fp,VXR->First,(int)VXR->Nentries)) return CRE;
	 if (!Read32s(fp,VXR->Last,(int)VXR->Nentries)) return CRE;
	 if (!Read32s(fp,VXR->Offset,(int)VXR->Nentries)) return CRE;
	 break;
       }
       case VXR_FIRSTREC:
       case VXR_LASTREC:
       case VXR_OFFSET: {
	 Int32 *buffer = va_arg (ap, Int32 *), nEntries;
	 long tOffset = offset + VXR_FIRSTREC_OFFSET;
	 if (!sX(ReadVXR(fp,offset,
			 VXR_NENTRIES,&nEntries,
			 VXR_NULL),&pStatus)) return pStatus;
	 switch (field) {
	   case VXR_FIRSTREC: break;
	   case VXR_LASTREC: tOffset += nEntries * sizeof(Int32); break;
	   case VXR_OFFSET: tOffset += 2 * nEntries * sizeof(Int32); break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32s(fp,buffer,(int)nEntries)) return CRE;
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case VXR_RECORDSIZE: tOffset += VXR_RECORDSIZE_OFFSET; break;
	   case VXR_RECORDTYPE: tOffset += VXR_RECORDTYPE_OFFSET; break;
	   case VXR_VXRNEXT: tOffset += VXR_VXRNEXT_OFFSET; break;
	   case VXR_NENTRIES: tOffset += VXR_NENTRIES_OFFSET; break;
	   case VXR_NUSEDENTRIES: tOffset += VXR_NUSEDENTRIES_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadVVR.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadVVR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadVVR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case VVR_NULL:
	 va_end (ap);
	 return pStatus;
       case VVR_RECORDx: {
	 struct VVRstruct *VVR = va_arg (ap, struct VVRstruct *);
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(VVR->RecordSize))) return CRE;
	 if (!Read32(fp,&(VVR->RecordType))) return CRE;
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case VVR_RECORDSIZE: tOffset += VVR_RECORDSIZE_OFFSET; break;
	   case VVR_RECORDTYPE: tOffset += VVR_RECORDTYPE_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadUIR.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadUIR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadUIR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case UIR_NULL:
	 va_end (ap);
	 return pStatus;
       case UIR_RECORD: {
	 struct UIRstruct *UIR = va_arg (ap, struct UIRstruct *);
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(UIR->RecordSize))) return CRE;
	 if (!Read32(fp,&(UIR->RecordType))) return CRE;
	 if (!Read32(fp,&(UIR->NextUIR))) return CRE;
	 if (!Read32(fp,&(UIR->PrevUIR))) return CRE;
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case UIR_RECORDSIZE: tOffset += UIR_RECORDSIZE_OFFSET; break;
	   case UIR_RECORDTYPE: tOffset += UIR_RECORDTYPE_OFFSET; break;
	   case UIR_NEXTUIR: tOffset += UIR_NEXTUIR_OFFSET; break;
	   case UIR_PREVUIR: tOffset += UIR_PREVUIR_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadCCR.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadCCR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadCCR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case CCR_NULL:
	 va_end (ap);
	 return pStatus;
       case CCR_RECORD: {
	 struct CCRstruct *CCR = va_arg (ap, struct CCRstruct *);
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(CCR->RecordSize))) return CRE;
	 if (!Read32(fp,&(CCR->RecordType))) return CRE;
	 if (!Read32(fp,&(CCR->CPRoffset))) return CRE;
	 if (!Read32(fp,&(CCR->uSize))) return CRE;
	 if (!Read32(fp,&(CCR->rfuA))) return CRE;
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case CCR_RECORDSIZE: tOffset += CCR_RECORDSIZE_OFFSET; break;
	   case CCR_RECORDTYPE: tOffset += CCR_RECORDTYPE_OFFSET; break;
	   case CCR_CPROFFSET: tOffset += CCR_CPROFFSET_OFFSET; break;
	   case CCR_USIZE: tOffset += CCR_USIZE_OFFSET; break;
	   case CCR_RFUa: tOffset += CCR_RFUa_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadCPR.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadCPR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadCPR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case CPR_NULL:
	 va_end (ap);
	 return pStatus;
       case CPR_RECORD: {
	 struct CPRstruct *CPR = va_arg (ap, struct CPRstruct *); int i;
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(CPR->RecordSize))) return CRE;
	 if (!Read32(fp,&(CPR->RecordType))) return CRE;
	 if (!Read32(fp,&(CPR->cType))) return CRE;
	 if (!Read32(fp,&(CPR->rfuA))) return CRE;
	 if (!Read32(fp,&(CPR->pCount))) return CRE;
	 if (CPR->pCount > CDF_MAX_PARMS) return TOO_MANY_PARMS;
	 for (i = 0; i < CPR->pCount; i++) {
	    if (!Read32(fp,&(CPR->cParms[i]))) return CRE;
	 }
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case CPR_RECORDSIZE: tOffset += CPR_RECORDSIZE_OFFSET; break;
	   case CPR_RECORDTYPE: tOffset += CPR_RECORDTYPE_OFFSET; break;
	   case CPR_CTYPE: tOffset += CPR_CTYPE_OFFSET; break;
	   case CPR_RFUa: tOffset += CPR_RFUa_OFFSET; break;
	   case CPR_PCOUNT: tOffset += CPR_PCOUNT_OFFSET; break;
	   case CPR_CPARM1: tOffset += CPR_CPARM1_OFFSET; break;
	   case CPR_CPARM2: tOffset += CPR_CPARM2_OFFSET; break;
	   case CPR_CPARM3: tOffset += CPR_CPARM3_OFFSET; break;
	   case CPR_CPARM4: tOffset += CPR_CPARM4_OFFSET; break;
	   case CPR_CPARM5: tOffset += CPR_CPARM5_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadSPR.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadSPR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadSPR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case SPR_NULL:
	 va_end (ap);
	 return pStatus;
       case SPR_RECORD: {
	 struct SPRstruct *SPR = va_arg (ap, struct SPRstruct *); int i;
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(SPR->RecordSize))) return CRE;
	 if (!Read32(fp,&(SPR->RecordType))) return CRE;
	 if (!Read32(fp,&(SPR->sArraysType))) return CRE;
	 if (!Read32(fp,&(SPR->rfuA))) return CRE;
	 if (!Read32(fp,&(SPR->pCount))) return CRE;
	 if (SPR->pCount > CDF_MAX_PARMS) return TOO_MANY_PARMS;
	 for (i = 0; i < SPR->pCount; i++) {
	    if (!Read32(fp,&(SPR->sArraysParms[i]))) return CRE;
	 }
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case SPR_RECORDSIZE: tOffset += SPR_RECORDSIZE_OFFSET; break;
	   case SPR_RECORDTYPE: tOffset += SPR_RECORDTYPE_OFFSET; break;
	   case SPR_STYPE: tOffset += SPR_STYPE_OFFSET; break;
	   case SPR_RFUa: tOffset += SPR_RFUa_OFFSET; break;
	   case SPR_PCOUNT: tOffset += SPR_PCOUNT_OFFSET; break;
	   case SPR_SPARM1: tOffset += SPR_SPARM1_OFFSET; break;
	   case SPR_SPARM2: tOffset += SPR_SPARM2_OFFSET; break;
	   case SPR_SPARM3: tOffset += SPR_SPARM3_OFFSET; break;
	   case SPR_SPARM4: tOffset += SPR_SPARM4_OFFSET; break;
	   case SPR_SPARM5: tOffset += SPR_SPARM5_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}

/******************************************************************************
* ReadCVVR.
******************************************************************************/

#if defined(STDARG)
STATICforIDL CDFstatus ReadCVVR (vFILE *fp, Int32 offset, ...)
#else
STATICforIDL CDFstatus ReadCVVR (va_alist)
va_dcl
#endif
{
  va_list ap; CDFstatus pStatus = CDF_OK;
#if defined(STDARG)
  va_start (ap, offset);
#else
  vFILE *fp; Int32 offset;
  VA_START (ap);
  fp = va_arg (ap, vFILE *);
  offset = va_arg (ap, Int32);
#endif
  for (;;) {
     int field = va_arg (ap, int);
     switch (field) {
       case CVVR_NULL:
	 va_end (ap);
	 return pStatus;
       case CVVR_RECORDx: {
	 struct CVVRstruct *CVVR = va_arg (ap, struct CVVRstruct *);
	 if (!SEEKv(fp,(long)offset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,&(CVVR->RecordSize))) return CRE;
	 if (!Read32(fp,&(CVVR->RecordType))) return CRE;
	 if (!Read32(fp,&(CVVR->rfuA))) return CRE;
	 if (!Read32(fp,&(CVVR->cSize))) return CRE;
	 break;
       }
       default: {
	 Int32 *buffer = va_arg (ap, Int32 *); long tOffset = offset;
	 switch (field) {
	   case CVVR_RECORDSIZE: tOffset += CVVR_RECORDSIZE_OFFSET; break;
	   case CVVR_RECORDTYPE: tOffset += CVVR_RECORDTYPE_OFFSET; break;
	   case CVVR_RFUa: tOffset += CVVR_RFUa_OFFSET; break;
	   case CVVR_CSIZE: tOffset += CVVR_CSIZE_OFFSET; break;
	   default: return CDF_INTERNAL_ERROR;
	 }
	 if (!SEEKv(fp,tOffset,vSEEK_SET)) return CRE;
	 if (!Read32(fp,buffer)) return CRE;
	 break;
       }
     }
  }
}
