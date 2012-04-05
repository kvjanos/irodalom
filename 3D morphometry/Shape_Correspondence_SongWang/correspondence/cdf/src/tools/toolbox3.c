/******************************************************************************
*
*  NSSDC/CDF              Toolbox of routines for CDF Toolkit (Miscellaneous).
*
*  Version 1.6b, 3-Mar-97, Hughes STX.
*
*  Modification history:
*
*   V1.0  25-Jan-94, J Love     Original version.
*   V1.0a  6-Feb-94, J Love     DEC Alpha/OpenVMS port.
*   V1.1  10-Mar-94, J Love     Modified `ParseOptionList' to make the
*                               enclosing parentheses optional (on any
*                               operating system).
*   V1.2  13-Dec-94, J Love     CDF V2.5.
*   V1.2a 13-Jan-95, J Love     Changed `IsCDF' to allow all possible
*                               extensions on Macintosh machines also.
*   V1.2b 20-Jan-95, J Love     IRIX 6.x (64-bit).
*   V1.2c  3-Mar-95, J Love     Moved `EncodeKeyDefinitions', etc. to
*                               `windoz.c'.
*   V1.3  21-Mar-95, J Love     POSIX.
*   V1.3a 18-Apr-95, J Love     More POSIX.
*   V1.4  22-May-95, J Love     `ASSIGNx', `EQx', etc. moved here (and some
*                               calling sequences changed).
*   V1.5  14-Sep-95, J Love     Added routines used by CDFexport.  Hyper
*                               groups.
*   V1.5a 19-Sep-95, J Love     Macintosh event handling.
*   V1.6   9-Sep-96, J Love     CDF V2.6.
*   V1.6a 21-Feb-97, J Love	Removed RICE.
*   V1.6b  3-Mar-97, J Love	Windows NT for MS Visual C/C++ on an IBM PC.
*   V1.6c 18-Dec-98, M Liu      Add the missing cases for ALPHAVMSi in
*                               WhichEncoding and EncodingToken.
*   V1.7  05-May-04, M Liu      Changed function AllocateBuffers when the
*                               record count is 1 or more.
*   V1.8  11-Jul-05, M Liu      Added MingW port for PC.
*
******************************************************************************/

#include "cdftools.h"

/******************************************************************************
* Macros.
******************************************************************************/

#if defined(dos)
#define MAXnBYTESperALLOCATION  50000L  /* Remember 64Kb limit. */
#else
#if defined(mac)
#define MAXnBYTESperALLOCATION  50000L
#else
#define MAXnBYTESperALLOCATION  500000L
#endif
#endif

/******************************************************************************
* Local function prototypes.
******************************************************************************/

static Logical EqStringsIgLTws PROTOARGs((char *string1, char *string2));
static Logical DecodeInt32 PROTOARGs((char *, Int32 *));
static Logical DecodeInt32u PROTOARGs((char *, uInt32 *));
static char *strnchr PROTOARGs((char *string, int chr, int n));
static QOP *ParseSlashStyle PROTOARGs((
  int argc, char *argv[], char *validQuals[], int optRequired[]
));
static QOP *ParseHyphenStyle PROTOARGs((
  int argc, char *argv[], char *validQuals[], int optRequired[]
));
static void FreeArgvT PROTOARGs((int lastArgN, char *argvT[]));
static QOP *BuildQOP PROTOARGs((
  int Nparms, char *parms[], char *validQuals[], int *qualEntered,
  char *qualOpt[]
));
static Logical AttemptAllocations PROTOARGs((
  long nScalarValues, int nScalars, long nHyperValues, int nHypers,
  Byte **handles[], size_t nValueBytes[]
));

/******************************************************************************
* EncodeRecordJustify.
******************************************************************************/

void EncodeRecordJustify (string, recN, minWidth)
char *string;
long recN;
int minWidth;   /* If zero, no minimum width.  If positive, right justified.
		   If negative, left justified. */
{
  sprintf (string, "%ld", recN + 1);
  Justify (string, minWidth);
  return;
}

/******************************************************************************
* EncodeIndicesJustify.
******************************************************************************/

void EncodeIndicesJustify (string, numDims, indices, minWidth)
char *string;
long numDims;
long indices[];
int minWidth;   /* If zero, no minimum width.  If positive, right justified.
		   If negative, left justified. */
{
  int n;
  strcpyX (string, "[", 0);
  for (n = 0; n < numDims; n++) {
     if (n > 0) strcatX (string, ",", 0);
     sprintf (EofS(string), "%ld", indices[n] + 1);
  }
  strcatX (string, "]", 0);
  Justify (string, minWidth);
  return;
}

/******************************************************************************
* EncodeRecordIndicesJustify.
******************************************************************************/

void EncodeRecordIndicesJustify (string, recN, numDims, indices, minWidth)
char *string;
long recN;
long numDims;
long indices[];
int minWidth;   /* If zero, no minimum width.  If positive, right justified.
		   If negative, left justified. */
{
  EncodeRecordJustify (string, recN, 0);
  EncodeIndicesJustify (EofS(string), numDims, indices, 0);
  Justify (string, minWidth);
  return;
}

/******************************************************************************
* Justify.
******************************************************************************/

void Justify (string, minWidth)
char *string;
int minWidth;   /* If zero, no minimum width.  If positive, right justified.
		   If negative, left justified. */
{
  if (minWidth < 0) {
    int pad = -minWidth - strlen(string);
    if (pad > 0) CatNcharacters (string, pad, (int) ' ');
  }
  else {
    if (minWidth > 0) {
      int i, stringL = strlen(string);
      int shift = minWidth - stringL;
      if (shift > 0) {
	memmove (&(string[shift]), string, stringL + 1);
	for (i = 0; i < shift; i++) string[i] = ' ';
      }
    }
  }
  return;
}

/******************************************************************************
* CatString.
******************************************************************************/

void CatToString (string, cat, length, justify, more)
char *string;
char *cat;
int length;
int justify;
char *more;
{
  int catL = (int) strlen (cat);
  int moreL = (int) strlen (more);
  if (catL > length)
    if (moreL >= length)
      CatNcharacters (string, length, '*');
    else {
      strcatX (string, cat, strlen(string) + (length - moreL));
      strcatX (string, more, 0);
    }
  else
    switch (justify) {
      case LEFT_JUSTIFY:
	strcatX (string, cat, 0);
	CatNcharacters (string, length - catL, ' ');
	break;
      case CENTER_JUSTIFY: {
	int after = (length - catL) / 2;
	int before = (length - catL) - after;
	CatNcharacters (string, before, ' ');
	strcatX (string, cat, 0);
	CatNcharacters (string, after, ' ');
	break;
      }
      case RIGHT_JUSTIFY:
	CatNcharacters (string, length - catL, ' ');
	strcatX (string, cat, 0);
	break;
    }
  return;
}

/******************************************************************************
* RemoveLeadingBlanks.
******************************************************************************/

void RemoveLeadingBlanks (string)
char *string;
{
  while (string[0] == ' ') memmove (string, &string[1], strlen(string));
  return;
}

/******************************************************************************
* RemoveTrailingBlanks.
******************************************************************************/

void RemoveTrailingBlanks (string)
char *string;
{
  int i;
  for (i = strlen(string) - 1; i >= 0; i--) {
     if (string[i] == ' ') string[i] = NUL;
  }
  return;
}

/******************************************************************************
* RemoveWhiteSpace.
******************************************************************************/

void RemoveWhiteSpace (string)
char *string;
{
  char *c = string;
  while (*c != NUL) {
    if (Spacing(*c))
      memmove (c, c + 1, strlen(c+1) + 1);
    else
      c++;
  }
  return;
}

/******************************************************************************
* CopyToField.
*    Copies a string to a field and blank pads to the end of the field.
******************************************************************************/

void CopyToField (string, field, length)
char *string;
char *field;
int length;
{
  strcpyX (field, string, length);
  BlankPadRight (field, length - strlen(string));
  return;
}

/******************************************************************************
* CopyFromField.
*    Copies from a field to a string and removes trailing blanks.  It is
* assumed that the string is large enough.
******************************************************************************/

void CopyFromField (field, length, string)
char *field;
int length;
char *string;
{
  strcpyX (string, field, length);
  RemoveTrailingBlanks (string);
  return;
}

/******************************************************************************
* SystemClock.
******************************************************************************/

double SystemClock () {
  return ((double) time(NULL));
}

/******************************************************************************
* ParsePath.
******************************************************************************/

void ParsePath (path, dir, name)
char *path;
char dir[DU_MAX_DIR_LEN+1];
char name[DU_MAX_NAME_LEN+1];
{
  char *cp;
  size_t len;
  char dirT[DU_MAX_DIR_LEN+1];
  char nameT[DU_MAX_NAME_LEN+1];
#if (defined(unix) && !defined(__CYGWIN__) && !defined(__MINGW32__)) || \
    defined(posixSHELL)
  if (IsDir(path)) {
    strcpyX (dirT, path, DU_MAX_DIR_LEN);
    nameT[0] = NUL;
  }
  else {
    cp = strrchr (path, '/');
    if (cp == NULL) {
      dirT[0] = NUL;
      strcpyX (nameT, path, DU_MAX_NAME_LEN);
    }
    else {
      len = cp - path;                          /* No trailing '/'. */
      strcpyX (dirT, path, MINIMUM(len,DU_MAX_DIR_LEN));
      strcpyX (nameT, cp + 1, DU_MAX_NAME_LEN);
    }
  }
#endif
#if defined(dos) || defined(win32) || defined(__CYGWIN__) || \
    defined(__MINGW32__)
  if (IsDir(path)) {
    strcpyX (dirT, path, DU_MAX_DIR_LEN);
    nameT[0] = NUL;
  }
  else {
    char *cp1 = strrchr (path, ':');
    char *cp2 = strrchr (path, '\\');
    cp = (cp1 > cp2 ? cp1 : cp2);
    if (cp == NULL) {
      dirT[0] = NUL;
      strcpyX (nameT, path, DU_MAX_NAME_LEN);
    }
    else {
      if (cp1 == NULL) {
	len = (size_t) (cp2 - path);            /* No trailing '\'. */
	strcpyX (dirT, path, MINIMUM(len,DU_MAX_DIR_LEN));
      }
      else {
	if (cp2 == NULL) {
	  len = (size_t) (cp1 - path + 1);      /* Trailing ':'. */
	  strcpyX (dirT, path, MINIMUM(len,DU_MAX_DIR_LEN));
	}
	else {
	  if (cp1 + 1 == cp2)
	    len = (size_t) (cp2 - path + 1);    /* Trailing ':'. */
	  else
	    len = (size_t) (cp2 - path);        /* No trailing ':'. */
	  strcpyX (dirT, path, MINIMUM(len,DU_MAX_DIR_LEN));
	}
      }
      strcpyX (nameT, cp + 1, DU_MAX_NAME_LEN);
    }
  }
#endif
#if defined(vms)
  char *cp1 = strrchr (path, ':');
  char *cp2 = strrchr (path, ']');
  cp = (cp1 > cp2 ? cp1 : cp2);
  if (cp == NULL) {
    dirT[0] = NUL;
    strcpyX (nameT, path, DU_MAX_NAME_LEN);
  }
  else {
    len = cp - path + 1;                        /* Trailing ':' or ']' */
    strcpyX (dirT, path, MINIMUM(len,DU_MAX_DIR_LEN));
    strcpyX (nameT, cp + 1, DU_MAX_NAME_LEN);
  }
#endif
#if defined(mac)
  if (IsDir(path)) {
    strcpyX (dirT, path, DU_MAX_DIR_LEN);
    nameT[0] = NUL;
  }
  else {
    cp = strrchr (path, ':');
    if (cp == NULL) {
      dirT[0] = NUL;
      strcpyX (nameT, path, DU_MAX_NAME_LEN);
    }
    else {
      len = cp - path + 1;                      /* Trailing ':'. */
      strcpyX (dirT, path, MINIMUM(len,DU_MAX_DIR_LEN));
      strcpyX (nameT, cp + 1, DU_MAX_NAME_LEN);
    }
  }
#endif
  if (dir != NULL) strcpyX (dir, dirT, DU_MAX_DIR_LEN);
  if (name != NULL) strcpyX (name, nameT, DU_MAX_NAME_LEN);
  return;
}

/******************************************************************************
* IsDir (directory).
* `stat' on MS-DOS systems will indicate a directory if the pathname
* contains a wildcard character.
******************************************************************************/

Logical IsDir (path)
char *path;
{
  char pathX[DU_MAX_PATH_LEN+1];
  size_t len;
#if (!defined(unix) && !defined(posixSHELL)) || defined(__CYGWIN__) || \
    defined(__MINGW32__)
  int lastChar;
#endif
#if defined(unix) || defined(dos) || defined(posixSHELL) || defined(win32)
  struct stat st;
#endif
  ExpandPath (path, pathX);
  len = strlen (pathX);
  if (len == 0) return FALSE;
#if (!defined(unix) && !defined(posixSHELL)) || defined(__CYGWIN__) || \
     defined(__MINGW32__)
  lastChar = len - 1;
#endif
#if defined(unix) || defined(dos) || defined(posixSHELL) || defined(win32)
  /****************************************************************************
  * UNIX/POSIXshell, DOS, and Windows.
  ****************************************************************************/
#if defined(dos) || defined(win32) || defined(__CYGWIN__) || \
    defined(__MINGW32__)
  /****************************************************************************
  * DOS/Windows: Check for wildcard character in the pathname.
  ****************************************************************************/
  if (strchr(pathX,'*') != NULL) return FALSE;
  if (strchr(pathX,'?') != NULL) return FALSE;
  /****************************************************************************
  * DOS/Windows: Strip trailing backslash (if present).
  ****************************************************************************/
  if (pathX[lastChar] == '\\') {
    pathX[lastChar] = NUL;         /* Strip trailing '\' */
    len--;
  }
  /****************************************************************************
  * DOS/Windows: Check for disk drive specification.
  ****************************************************************************/
  if (len == 2)
    if (Alphabetic(pathX[0]) && pathX[1] == ':') return TRUE;
#endif
#if (defined(unix) && !defined(__CYGWIN__) && !defined(__MINGW32__)) || \
    defined(posixSHELL)
  /****************************************************************************
  * UNIX: Check for wildcard character in the pathname.
  ****************************************************************************/
  if (strchr(pathX,'*') != NULL) return FALSE;
  if (strchr(pathX,'?') != NULL) return FALSE;
#endif
  /****************************************************************************
  * Call `stat' to determine type of file.
  ****************************************************************************/
  if (stat(pathX, &st) == 0) {
#if defined(SALFORDC)           /* Salford's `stat' is broken. */
    return (st.st_size == 0);
#else
#if defined(S_ISDIR)
    return S_ISDIR(st.st_mode);
#else
    return (st.st_mode & S_IFDIR);
#endif
#endif
  }
  else
    return FALSE;
#endif
#if defined(vms)
  /****************************************************************************
  * VMS.
  ****************************************************************************/
  /****************************************************************************
  * Check for wildcard character in the pathname.
  ****************************************************************************/
  if (strchr(pathX,'*') != NULL) return FALSE;
  if (strchr(pathX,'%') != NULL) return FALSE;
  /****************************************************************************
  * Check for possible logical name defined to be a full file specification
  * (not just a directory).
  ****************************************************************************/
  if (IsReg(pathX))
    return FALSE;
  else
    /**************************************************************************
    * If not, check if the path ends with a ':' (logical name) or ']'
    * (directory).
    **************************************************************************/
    if (pathX[lastChar] == ':' || pathX[lastChar] == ']')
      return TRUE;
    else
      return FALSE;
#endif
#if defined(mac)
  /****************************************************************************
  * MAC: Check to see if specification ends with a `:' (and assume it to be a
  * directory without actually checking).  If specification doesn't end with
  * a `:', check if it is a directory anyway.
  ****************************************************************************/
  if (pathX[lastChar] == ':')
    return TRUE;
  else {
    return MacDirSpecified (pathX, NULL, NULL);
  }
#endif
}

/******************************************************************************
* IsWild (Wildcard specification in name).
******************************************************************************/

Logical IsWild (path)
char *path;
{
  char dir[DU_MAX_DIR_LEN+1];
  char name[DU_MAX_NAME_LEN+1];
  ParsePath (path, dir, name);
  if (strchr(name,'*') != NULL) return TRUE;
#if defined(vms)
  if (strchr(name,'%') != NULL) return TRUE;
#else
  if (strchr(name,'?') != NULL) return TRUE;
#endif
  return FALSE;
}

/******************************************************************************
* IsCDF.
*    Determines if the specified pathname is that of a CDF.  Because of
* inconsistent behavior of CD-ROM drivers on various UNIX/POSIXshell machines
* and the Macintosh, the following extensions are tried: `.cdf', `.CDF',
* `.cdf;1', and `.CDF;1'.
******************************************************************************/

Logical IsCDF (pathname)
char *pathname;
{
  char pathnameX[DU_MAX_PATH_LEN+1];
  ExpandPath (pathname, pathnameX);
  if (IsReg(pathnameX)) return TRUE;
  strcatX (pathnameX, ".cdf", DU_MAX_PATH_LEN);
  if (IsReg(pathnameX)) return TRUE;
#if defined(unix) || defined(mac) || defined(posixSHELL)
  ExpandPath (pathname, pathnameX);
  strcatX (pathnameX, ".cdf;1", DU_MAX_PATH_LEN);
  if (IsReg(pathnameX)) return TRUE;
  ExpandPath (pathname, pathnameX);
  strcatX (pathnameX, ".CDF", DU_MAX_PATH_LEN);
  if (IsReg(pathnameX)) return TRUE;
  ExpandPath (pathname, pathnameX);
  strcatX (pathnameX, ".CDF;1", DU_MAX_PATH_LEN);
  if (IsReg(pathnameX)) return TRUE;
#endif
  return FALSE;
}

/******************************************************************************
* Qop.
******************************************************************************/

QOP *Qop (argc, argv, validQuals, optRequired)
int argc;
char *argv[];
char *validQuals[];
int optRequired[];
{
  /****************************************************************************
  * Check to see if a style definition has been specified.  (A slash or hyphen
  * as the first command line argument.)
  ****************************************************************************/
  if (argc > 1) {
    if (!strcmp(argv[1],"/")) return ParseSlashStyle (argc - 1, &argv[1],
						      validQuals, optRequired);
    if (!strcmp(argv[1],"-")) return ParseHyphenStyle (argc - 1, &argv[1],
						       validQuals,optRequired);
  }
  /****************************************************************************
  * No style definition, parse according to default for operating system
  * being used.
  ****************************************************************************/
#if defined(vms)
  return ParseSlashStyle (argc, argv, validQuals, optRequired);
#else
  return ParseHyphenStyle (argc, argv, validQuals, optRequired);
#endif
}

/******************************************************************************
* ParseSlashStyle.
* Assume qualifiers begin with a slash (/).
******************************************************************************/

static QOP *ParseSlashStyle (argc, argv, validQuals, optRequired)
int argc;
char *argv[];
char *validQuals[];
int optRequired[];
{
  QOP *qop;                     /* Pointer to QOP structure. */
  int argN,                     /* Argument number. */
      qualNv,                   /* Qualifier number (from list of valid
				   qualifiers). */
      qualNe,                   /* Qualifier number (from list of qualifiers
				   entered). */
      Nparms = 0,               /* Number of parameters entered. */
      Nquals = 0,               /* Number of qualifiers entered. */
      matchQualNv,              /* Matching qualifier number (from list of
				   valid qualifiers). */
      qualEntered[QOP_MAX_QUALs];
				/* TRUE if the corresponding qualifier was
				   entered. */
  char *parms[QOP_MAX_PARMs],   /* Pointers to the parameters entered. */
       *qualOpt[QOP_MAX_QUALs], /* Pointers to the options of the qualifiers
				   entered (indexed to correspond to the list
				   of valid qualifiers). */
       *argvT[QOP_MAX_ARGVs],   /* Pointers to temporary `argv' strings. */
       *quals[QOP_MAX_QUALs],   /* Pointers to the qualifiers entered (in the
				   order they were entered). */
       *opts[QOP_MAX_QUALs],    /* Pointers to the options entered (in the
				   order they were entered). */
       *ptr;                    /* Pointer into character string. */
  enum modes { UNKNOWN_,
	       START_OF_PARM_,
	       FIND_PARM_END_,
	       START_OF_QUAL_,
	       FIND_QUAL_END_,
	       START_OF_OPT_,
	       FIND_OPT_END_ } mode;  /* The current mode when scanning the
					 list of command line arguments. */
  /****************************************************************************
  * Determine parameters and qualifiers/options.
  ****************************************************************************/
  for (argN = 1; argN < argc; argN++) {
     char nChars = strlen (argv[argN]);
     argvT[argN] = (char *) cdf_AllocateMemory (nChars + 1, FatalError);
     strcpyX (argvT[argN], argv[argN], nChars);
     mode = UNKNOWN_;
     for (ptr = argvT[argN]; *ptr != NUL;) {
	switch (mode) {
	  case UNKNOWN_:
	    if (*ptr == '/') {
	      mode = START_OF_QUAL_;
	      ptr++;
	    }
	    else
	      mode = START_OF_PARM_;
	    break;
	  case START_OF_PARM_:
	    Nparms++;
	    if (Nparms <= QOP_MAX_PARMs)
	      parms[Nparms-1] = ptr;
	    else {
	      DisplayError ("Too many parameters.");
	      FreeArgvT (argN, argvT);
	      return NULL;
	    }
	    mode = FIND_PARM_END_;
	    ptr++;
	    break;
	  case FIND_PARM_END_:
	    if (*ptr == '/')
	      if (*(ptr+1) == '/')
		memmove (ptr, ptr+1, strlen(ptr+1) + 1);
	      else {
		*ptr = NUL;
		mode = START_OF_QUAL_;
	      }
	    ptr++;
	    break;
	  case START_OF_QUAL_:
	    if (*ptr == '/')
	      mode = START_OF_PARM_;
	    else {
	      Nquals++;
	      if (Nquals <= QOP_MAX_QUALs)
		quals[Nquals-1] = ptr;
	      else {
		DisplayError ("Too many qualifiers.");
		FreeArgvT (argN, argvT);
		return NULL;
	      }
	      mode = FIND_QUAL_END_;
	      ptr++;
	    }
	    break;
	  case FIND_QUAL_END_:
	    switch (*ptr) {
	      case '/':
		*ptr = NUL;
		opts[Nquals-1] = NULL;
		mode = START_OF_QUAL_;
		break;
	      case '=':
		*ptr = NUL;
		mode = START_OF_OPT_;
		break;
	    }
	    ptr++;
	    break;
	  case START_OF_OPT_:
	    if (*ptr == '/')
	      if (*(ptr+1) == '/') {
		opts[Nquals-1] = ptr + 1;
		ptr += 2;
		mode = FIND_OPT_END_;
	      }
	      else {
		char tempS[MAX_MESSAGE_TEXT_LEN+1];
		sprintf (tempS, "Missing option for qualifier `/%s'.",
				 quals[Nquals-1]);
		DisplayError (tempS);
		FreeArgvT (argN, argvT);
		return NULL;
	      }
	    else {
	      opts[Nquals-1] = ptr;
	      mode = FIND_OPT_END_;
	      ptr++;
	    }
	    break;
	  case FIND_OPT_END_:
	    if (*ptr == '/')
	      if (*(ptr+1) == '/')
		memmove (ptr, ptr+1, strlen(ptr+1) + 1);
	      else {
		*ptr = NUL;
		mode = START_OF_QUAL_;
	      }
	    ptr++;
	    break;
	}
     }
     switch (mode) {
       case UNKNOWN_:
	 /* Logic error. */
	 break;
       case START_OF_PARM_:
	 /* Logic error. */
	 break;
       case START_OF_QUAL_:
	 DisplayError ("Missing qualifier body (/).");
	 FreeArgvT (argN, argvT);
	 return NULL;
       case FIND_QUAL_END_:
	 /* Qualifier already NUL-terminated */
	 opts[Nquals-1] = NULL;
	 break;
       case FIND_PARM_END_:
	 /* Parameter already NUL-terminated */
	 break;
       case START_OF_OPT_: {
	 char tempS[MAX_MESSAGE_TEXT_LEN+1];
	 sprintf (tempS,"Missing option for qualifier `/%s'.",quals[Nquals-1]);
	 DisplayError (tempS);
	 FreeArgvT (argN, argvT);
	 return NULL;
       }
       case FIND_OPT_END_:
	 /* Option already NUL-terminated */
	 break;
     }
  }
  /****************************************************************************
  * Set up to scan command line arguments (while checking number of valid
  * qualifiers).
  ****************************************************************************/
  for (qualNv = 0; validQuals[qualNv] != NULL; qualNv++) {
     if (qualNv < QOP_MAX_QUALs) {
       qualEntered[qualNv] = FALSE;
       qualOpt[qualNv] = NULL;
     }
     else {
       WriteOut (stdout, "Too many valid qualifiers.\n");
       FreeArgvT (argc - 1, argvT);
       return NULL;
     }
  }
  /****************************************************************************
  * Determine which qualifiers/options were entered.
  ****************************************************************************/
  for (qualNe = 0; qualNe < Nquals; qualNe++) {
     matchQualNv = FindUniqueMatch (quals[qualNe], validQuals);
     switch (matchQualNv) {
       case NOMATCH: {
	 char tempS[MAX_MESSAGE_TEXT_LEN+1];
	 sprintf (tempS, "Unknown qualifier (/%s).", quals[qualNe]);
	 DisplayError (tempS);
	 FreeArgvT (argc - 1, argvT);
	 return NULL;
       }
       case MATCHES: {
	 char tempS[MAX_MESSAGE_TEXT_LEN+1];
	 sprintf (tempS, "Ambiguous qualifier (/%s).", quals[qualNe]);
	 DisplayError (tempS);
	 FreeArgvT (argc - 1, argvT);
	 return NULL;
       }
       default: {
	 if (optRequired[matchQualNv] && opts[qualNe] == NULL) {
	   char tempS[MAX_MESSAGE_TEXT_LEN+1];
	   sprintf (tempS, "Option missing for qualifier `/%s'.",
		    validQuals[matchQualNv]);
	   DisplayError (tempS);
	   FreeArgvT (argc - 1, argvT);
	   return NULL;
         }
	 if (!optRequired[matchQualNv] && opts[qualNe] != NULL) {
	   char tempS[MAX_MESSAGE_TEXT_LEN+1];
	   sprintf (tempS, "Extraneous option for qualifier `/%s'.",
		    validQuals[matchQualNv]);
	   DisplayError (tempS);
	   FreeArgvT (argc - 1, argvT);
	   return NULL;
	 }
	 if (qualEntered[matchQualNv]) {
	   char tempS[MAX_MESSAGE_TEXT_LEN+1];
	   sprintf (tempS, "Redundant qualifier (/%s).",
		    validQuals[matchQualNv]);
	   DisplayError (tempS);
	   FreeArgvT (argc - 1, argvT);
	   return NULL;
	 }
	 qualEntered[matchQualNv] = TRUE;
	 qualOpt[matchQualNv] = opts[qualNe];
	 break;
       }
     }
  }
  /****************************************************************************
  * Build QOP structure.
  ****************************************************************************/
  qop = BuildQOP (Nparms, parms, validQuals, qualEntered, qualOpt);
  /****************************************************************************
  * Free memory used and return QOP structure.
  ****************************************************************************/
  FreeArgvT (argc - 1, argvT);
  return qop;
}

/******************************************************************************
* ParseHyphenStyle.
* Assume qualifiers begin with a hyphen (-).
******************************************************************************/

static QOP *ParseHyphenStyle (argc, argv, validQuals, optRequired)
int argc;
char *argv[];
char *validQuals[];
int optRequired[];
{
  QOP *qop;                     /* Pointer to a QOP structure. */
  int argN,                     /* Command line argument number. */
      qualNv,                   /* Qualifier number from `valids' list. */
      Nparms = 0,               /* Number of parameters entered. */
      matchQualNv,              /* Matching qualifier number (from list of
				   valid qualifiers). */
      qualEntered[QOP_MAX_QUALs];
				/* TRUE if the corresponding qualifier was
				   entered. */
  char *parms[QOP_MAX_PARMs],   /* Pointers to the parameters entered. */
       *qualOpt[QOP_MAX_QUALs]; /* Pointers to the options of the qualifiers
				   entered (indexed to correspond to the list
				   of valid qualifiers). */
  /****************************************************************************
  * Set up to scan command line arguments (while checking number of valid
  * qualifiers).
  ****************************************************************************/
  for (qualNv = 0; validQuals[qualNv] != NULL; qualNv++) {
     if (qualNv < QOP_MAX_QUALs) {
       qualEntered[qualNv] = FALSE;
       qualOpt[qualNv] = NULL;
     }
     else {
       DisplayError ("Too many valid qualifiers.");
       return NULL;
     }
  }
  /****************************************************************************
  * Determine which qualifiers were entered (the rest being parameters).
  ****************************************************************************/
  for (argN = 1; argN < argc; argN++) {
     if (argv[argN][0] == '-' && argv[argN][1] != '-') {
       matchQualNv = FindUniqueMatch (&argv[argN][1], validQuals);
       switch (matchQualNv) {
	 case NOMATCH: {
	   char tempS[MAX_MESSAGE_TEXT_LEN+1];
	   sprintf (tempS, "Unknown qualifier (-%s).", &argv[argN][1]);
	   DisplayError (tempS);
	   return NULL;
	 }
	 case MATCHES: {
	   char tempS[MAX_MESSAGE_TEXT_LEN+1];
	   sprintf (tempS, "Ambiguous qualifier (-%s).", &argv[argN][1]);
	   DisplayError (tempS);
	   return NULL;
	 }
	 default: {
	   if (qualEntered[matchQualNv]) {
	     char tempS[MAX_MESSAGE_TEXT_LEN+1];
	     sprintf (tempS, "Redundant qualifier (-%s).",
		      validQuals[matchQualNv]);
	     DisplayError (tempS);
	     return NULL;
	   }
	   else
	     qualEntered[matchQualNv] = TRUE;
	   if (optRequired[matchQualNv]) {
	     if (argN+1 < argc) {
	       qualOpt[matchQualNv] = argv[argN+1];
	       argN++;
	     }
	     else {
	       char tempS[MAX_MESSAGE_TEXT_LEN+1];
	       sprintf (tempS, "Option missing for qualifier `-%s'.",
			validQuals[matchQualNv]);
	       DisplayError (tempS);
	       return NULL;
	     }
	   }
	 }
       }
     }
     else {
       Nparms++;
       if (Nparms <= QOP_MAX_PARMs)
	 parms[Nparms-1] = BOO(strncmp(argv[argN],"--",2),
			       argv[argN],argv[argN] + 1);
       else {
	 DisplayError ("Too many parameters.");
	 return NULL;
       }
     }
  }
  /****************************************************************************
  * Build QOP structure.
  ****************************************************************************/
  qop = BuildQOP (Nparms, parms, validQuals, qualEntered, qualOpt);
  /****************************************************************************
  * Return QOP structure.
  ****************************************************************************/
  return qop;
}

/******************************************************************************
* BuildQOP.
******************************************************************************/

static QOP *BuildQOP (Nparms, parms, validQuals, qualEntered, qualOpt)
int Nparms;
char *parms[];
char *validQuals[];
int *qualEntered;
char *qualOpt[];
{
  QOP *qop;             /* Pointer to a QOP structure. */
  int parmN,            /* Parameter number. */
      qualNv,           /* Qualifier number (in list of valid qualifiers. */
      strCount = 0;     /* Number of characters in the strings to be contained
			   in the QOP structure. */
  char *strOffset;      /* Offset into the QOP structure of a string
			   (parameter/option). */
  for (parmN = 0; parmN < Nparms; parmN++)
     strCount += strlen(parms[parmN]) + 1;
  for (qualNv = 0; validQuals[qualNv] != NULL; qualNv++)
     if (qualEntered[qualNv])
       if (qualOpt[qualNv] != NULL) strCount += strlen(qualOpt[qualNv]) + 1;
  qop = (QOP *) cdf_AllocateMemory (sizeof(QOP) + strCount,
				FatalError);
  strOffset = (char *) qop + sizeof(QOP);
  qop->Nparms = Nparms;
  for (parmN = 0; parmN < Nparms; parmN++) {
     qop->parms[parmN] = strOffset;
     strOffset += strlen(parms[parmN]) + 1;
     strcpyX (qop->parms[parmN], parms[parmN], 0);
  }
  for (qualNv = 0; validQuals[qualNv] != NULL; qualNv++) {
     qop->qualEntered[qualNv] = qualEntered[qualNv];
     if (qualEntered[qualNv]) {
       if (qualOpt[qualNv] != NULL) {
	 qop->qualOpt[qualNv] = strOffset;
	 strOffset += strlen(qualOpt[qualNv]) + 1;
	 strcpyX (qop->qualOpt[qualNv], qualOpt[qualNv], 0);
       }
       else
	 qop->qualOpt[qualNv] = NULL;
     }
  }
  return qop;
}

/******************************************************************************
* FreeArgvT.
******************************************************************************/

static void FreeArgvT (lastArgN, argvT)
int lastArgN;
char *argvT[];
{
  int argN;
  for (argN = 1; argN <= lastArgN; argN++) cdf_FreeMemory (argvT[argN],
						       FatalError);
  return;
}

/******************************************************************************
* ASSIGNx.
******************************************************************************/

void ASSIGNx (dst, src, dataType, numElems)
void *dst;
void *src;
long dataType;
long numElems;
{
  memmove (dst, src, (size_t) (CDFelemSize(dataType) * numElems));
  return;
}

/******************************************************************************
* EQx.
******************************************************************************/

Logical EQx (a, b, dataType, numElems)
void *a;
void *b;
long dataType;
long numElems;
{
  if (dataType != CDF_EPOCH16)
    return BOO(memcmp(a,b,(size_t)(CDFelemSize(dataType)*numElems)),FALSE,TRUE);
  else {
    double epoch16_a[2], epoch16_b[2];
    epoch16_a[0] = *((double *) a);
    epoch16_a[1] = *(((double *) a)+1);
    epoch16_b[0] = *((double *) b);
    epoch16_b[1] = *(((double *) b)+1);
    if (epoch16_a[0] == epoch16_b[0] && epoch16_a[1] == epoch16_b[1])
      return TRUE;
    return FALSE;
  }
}

/******************************************************************************
* NEx.
******************************************************************************/

Logical NEx (a, b, dataType, numElems)
void *a;
void *b;
long dataType;
long numElems;
{
  if (dataType != CDF_EPOCH16)
    return BOO(memcmp(a,b,(size_t)(CDFelemSize(dataType)*numElems)),TRUE,FALSE);
  else {
    double epoch16_a[2], epoch16_b[2];
    epoch16_a[0] = *((double *) a);
    epoch16_a[1] = *(((double *) a)+1);
    epoch16_b[0] = *((double *) b);
    epoch16_b[1] = *(((double *) b)+1);
    if (epoch16_a[0] == epoch16_b[0] && epoch16_a[1] == epoch16_b[1])
      return FALSE;
    return TRUE;
  }
}

/******************************************************************************
* LEx.
******************************************************************************/

Logical LEx (a, b, dataType, numElems)
void *a;
void *b;
long dataType;
long numElems;
{
switch (dataType) {
  case CDF_BYTE:
  case CDF_INT1:
    return (*((sChar *) a) <= *((sChar *) b));
  case CDF_UINT1:
    return (*((uChar *) a) <= *((uChar *) b));
  case CDF_INT2:
    return (*((Int16 *) a) <= *((Int16 *) b));
  case CDF_UINT2:
    return (*((uInt16 *) a) <= *((uInt16 *) b));
  case CDF_INT4:
    return (*((Int32 *) a) <= *((Int32 *) b));
  case CDF_UINT4:
    return (*((uInt32 *) a) <= *((uInt32 *) b));
  case CDF_REAL4:
  case CDF_FLOAT:
    return (*((float *) a) <= *((float *) b));
  case CDF_REAL8:
  case CDF_DOUBLE:
  case CDF_EPOCH:
    return (*((double *) a) <= *((double *) b));
  case CDF_EPOCH16: {
    double epoch16_a[2], epoch16_b[2];
    epoch16_a[0] = *((double *) a);
    epoch16_a[1] = *(((double *) a)+1);
    epoch16_b[0] = *((double *) b);
    epoch16_b[1] = *(((double *) b)+1);
    if (epoch16_a[0] > epoch16_b[0]) return FALSE;
    if (epoch16_a[0] == epoch16_b[0] && epoch16_a[1] > epoch16_b[1])
      return FALSE;
    return TRUE;
  }
  case CDF_CHAR:
  case CDF_UCHAR:
    return BOO(memcmp(a,b,(size_t)numElems) <= 0,TRUE,FALSE);
}
return FALSE;
}

/******************************************************************************
* LTx.
******************************************************************************/

Logical LTx (a, b, dataType, numElems)
void *a;
void *b;
long dataType;
long numElems;
{
switch (dataType) {
  case CDF_BYTE:
  case CDF_INT1:
    return (*((sChar *) a) < *((sChar *) b));
  case CDF_UINT1:
    return (*((uChar *) a) < *((uChar *) b));
  case CDF_INT2:
    return (*((Int16 *) a) < *((Int16 *) b));
  case CDF_UINT2:
    return (*((uInt16 *) a) < *((uInt16 *) b));
  case CDF_INT4:
    return (*((Int32 *) a) < *((Int32 *) b));
  case CDF_UINT4:
    return (*((uInt32 *) a) < *((uInt32 *) b));
  case CDF_REAL4:
  case CDF_FLOAT:
    return (*((float *) a) < *((float *) b));
  case CDF_REAL8:
  case CDF_DOUBLE:
  case CDF_EPOCH:
    return (*((double *) a) < *((double *) b));
  case CDF_EPOCH16: {
    double epoch16_a[2], epoch16_b[2];
    epoch16_a[0] = *((double *) a);
    epoch16_a[1] = *(((double *) a)+1);
    epoch16_b[0] = *((double *) b);
    epoch16_b[1] = *(((double *) b)+1);
    if (epoch16_a[0] > epoch16_b[0]) return FALSE;
    if (epoch16_a[0] == epoch16_b[0] && epoch16_a[1] >= epoch16_b[1])
      return FALSE;
    return TRUE;
  }
  case CDF_CHAR:
  case CDF_UCHAR:
    return BOO(memcmp(a,b,(size_t)numElems) < 0,TRUE,FALSE);
}
return FALSE;
}

/******************************************************************************
* GEx.
******************************************************************************/

Logical GEx (a, b, dataType, numElems)
void *a;
void *b;
long dataType;
long numElems;
{
switch (dataType) {
  case CDF_BYTE:
  case CDF_INT1:
    return (*((sChar *) a) >= *((sChar *) b));
  case CDF_UINT1:
    return (*((uChar *) a) >= *((uChar *) b));
  case CDF_INT2:
    return (*((Int16 *) a) >= *((Int16 *) b));
  case CDF_UINT2:
    return (*((uInt16 *) a) >= *((uInt16 *) b));
  case CDF_INT4:
    return (*((Int32 *) a) >= *((Int32 *) b));
  case CDF_UINT4:
    return (*((uInt32 *) a) >= *((uInt32 *) b));
  case CDF_REAL4:
  case CDF_FLOAT:
    return (*((float *) a) >= *((float *) b));
  case CDF_REAL8:
  case CDF_DOUBLE:
  case CDF_EPOCH:
    return (*((double *) a) >= *((double *) b));
  case CDF_EPOCH16: {
    double epoch16_a[2], epoch16_b[2];
    epoch16_a[0] = *((double *) a);
    epoch16_a[1] = *(((double *) a)+1);
    epoch16_b[0] = *((double *) b);
    epoch16_b[1] = *(((double *) b)+1);
    if (epoch16_a[0] < epoch16_b[0]) return FALSE;
    if (epoch16_a[0] == epoch16_b[0] && epoch16_a[1] < epoch16_b[1])
      return FALSE;
    return TRUE;
  }
  case CDF_CHAR:
  case CDF_UCHAR:
    return BOO(memcmp(a,b,(size_t)numElems) >= 0,TRUE,FALSE);
}
return FALSE;
}

/******************************************************************************
* GTx.
******************************************************************************/

Logical GTx (a, b, dataType, numElems)
void *a;
void *b;
long dataType;
long numElems;
{
switch (dataType) {
  case CDF_BYTE:
  case CDF_INT1:
    return (*((sChar *) a) > *((sChar *) b));
  case CDF_UINT1:
    return (*((uChar *) a) > *((uChar *) b));
  case CDF_INT2:
    return (*((Int16 *) a) > *((Int16 *) b));
  case CDF_UINT2:
    return (*((uInt16 *) a) > *((uInt16 *) b));
  case CDF_INT4:
    return (*((Int32 *) a) > *((Int32 *) b));
  case CDF_UINT4:
    return (*((uInt32 *) a) > *((uInt32 *) b));
  case CDF_REAL4:
  case CDF_FLOAT:
    return (*((float *) a) > *((float *) b));
  case CDF_REAL8:
  case CDF_DOUBLE:
  case CDF_EPOCH:
    return (*((double *) a) > *((double *) b));
  case CDF_EPOCH16: {
    double epoch16_a[2], epoch16_b[2];
    epoch16_a[0] = *((double *) a);
    epoch16_a[1] = *(((double *) a)+1);
    epoch16_b[0] = *((double *) b);
    epoch16_b[1] = *(((double *) b)+1);
    if (epoch16_a[0] < epoch16_b[0]) return FALSE;
    if (epoch16_a[0] == epoch16_b[0] && epoch16_a[1] <= epoch16_b[1])
      return FALSE;
    return TRUE;
  }
  case CDF_CHAR:
  case CDF_UCHAR:
    return BOO(memcmp(a,b,(size_t)numElems) > 0,TRUE,FALSE);
}
return FALSE;
}

/******************************************************************************
* IncrRecordIndicesFirstLast.
* Rolls over to `first' indices if at `last' indices (and increments record).
******************************************************************************/

void IncrRecordIndicesFirstLast (rowMajor, recordN, numDims, first, last,
				 indices)
Logical rowMajor;
long *recordN;
long numDims;
long first[];
long last[];
long indices[];
{
  if (numDims > 0) {
    int firstDim = BOO(rowMajor,(int)(numDims - 1),0);
    int lastDim = BOO(rowMajor,0,(int)(numDims - 1));
    int dimIncr = BOO(rowMajor,-1,1);
    int doneDim = lastDim + dimIncr, dimN;
    for (dimN = firstDim; dimN != doneDim; dimN += dimIncr) {
       if (indices[dimN] == last[dimN]) {
         indices[dimN] = first[dimN];
         if (dimN == lastDim) (*recordN)++;
       }
       else {
         indices[dimN]++;
         break;
       }
    }
  }
  else
    (*recordN)++;
  return;
}

/******************************************************************************
* IncrIndicesFirstLastRow.
* Rolls over to `first' indices if at `last' indices.
******************************************************************************/

void IncrIndicesFirstLastRow (numDims, first, last, indices)
long numDims;
long first[];
long last[];
long indices[];
{
  int dimN;
  for (dimN = (int) (numDims - 1); dimN >= 0; dimN--) {
     if (indices[dimN] == last[dimN])
       indices[dimN] = first[dimN];
     else {
       indices[dimN]++;
       break;
     }
  }
  return;
}

/******************************************************************************
* IncrIndicesFirstLastCol.
* Rolls over to `first' indices if at `last' indices.
******************************************************************************/

void IncrIndicesFirstLastCol (numDims, first, last, indices)
long numDims;
long first[];
long last[];
long indices[];
{
  int dimN;
  for (dimN = 0; dimN < numDims; dimN++) {
     if (indices[dimN] == last[dimN])
       indices[dimN] = first[dimN];
     else {
       indices[dimN]++;
       break;
     }
  }
  return;
}

/******************************************************************************
* AllocateBuffers.
******************************************************************************/

Logical AllocateBuffers (nRecords, numDims, dimSizes, groups, nScalarBuffers,
			 nHyperBuffers, handles, nValueBytes, rowMajor,
			 minNofHypers, fatalFnc)
long nRecords;
long numDims;
long dimSizes[];
struct GroupStruct *groups;
int nScalarBuffers;
int nHyperBuffers;
Byte **handles[];
size_t nValueBytes[];
Logical rowMajor;
int minNofHypers;
void (*fatalFnc) PROTOARGs((char *msg));
{
  int lsDimN;           /* Least significant dimension number. */
  int msDimN;           /* Most significant dimension number. */
  int dimStep;          /* How to increment dimension number from most
			   significant to least significant. */
  int dimN, dN;
  long nValuesPerRec, nValuesPerDim[CDF_MAX_DIMS], count, nHypersSoFar;
  /****************************************************************************
  * Setup for majority.
  ****************************************************************************/
  lsDimN = BOO(rowMajor,(int)(numDims-1),0);
  msDimN = BOO(rowMajor,0,(int)(numDims-1));
  dimStep = BOO(rowMajor,1,-1);
  /****************************************************************************
  * Calculate number of values per record/dimension.
  ****************************************************************************/
  if (numDims > 0) {
    nValuesPerDim[lsDimN] = 1;
    for (dimN = lsDimN - dimStep; dimN != msDimN - dimStep; dimN -= dimStep) {
       nValuesPerDim[dimN] = dimSizes[dimN+dimStep] *
			     nValuesPerDim[dimN+dimStep];
    }
    nValuesPerRec = dimSizes[msDimN] * nValuesPerDim[msDimN];
  }
  else
    nValuesPerRec = 1;
  /****************************************************************************
  * Check if full records can be allocated.
  ****************************************************************************/
  count = nRecords;
  while (count >= 1 ) {
     long nRecGroups = nRecords/count + BOO(nRecords % count == 0,0,1);
     if (nRecGroups >= minNofHypers || count == 1) {
       if (AttemptAllocations(count,nScalarBuffers,count*nValuesPerRec,
			      nHyperBuffers,handles,nValueBytes)) {
	 groups->nRecGroups = nRecGroups;
	 groups->firstRecCount = count;
	 groups->lastRecCount = BOO(nRecords % count == 0,
				    count,nRecords % count);
	 groups->recGroupN = 1;
	 for (dimN = msDimN; dimN != lsDimN + dimStep; dimN += dimStep) {
	    groups->nDimGroups[dimN] = 1;
	    groups->firstDimCount[dimN] = dimSizes[dimN];
	    groups->lastDimCount[dimN] = dimSizes[dimN];
	    groups->dimGroupN[dimN] = 1;
	 }
	 return TRUE;
       }
     }
     if (count == 1) break;
     count = (count + BOO(count % 2 == 0,0,1)) / 2;
  }
  groups->nRecGroups = nRecords;
  groups->firstRecCount = 1;
  groups->lastRecCount = 1;
  groups->recGroupN = 1;
  nHypersSoFar = nRecords;
  /****************************************************************************
  * Check how much of each dimension can be allocated.
  ****************************************************************************/
  for (dimN = msDimN; dimN != lsDimN + dimStep; dimN += dimStep) {
     count = dimSizes[dimN];
     while (count > 1) {
	long nDimGroups = (dimSizes[dimN] / count) +
			  BOO(dimSizes[dimN] % count == 0,0,1);
	if (nHypersSoFar * nDimGroups >= minNofHypers) {
	  if (AttemptAllocations(1L,nScalarBuffers,count*nValuesPerDim[dimN],
				 nHyperBuffers,handles,nValueBytes)) {
	    groups->nDimGroups[dimN] = nDimGroups;
	    groups->firstDimCount[dimN] = count;
	    groups->lastDimCount[dimN] = BOO(dimSizes[dimN] % count == 0,
					     count,dimSizes[dimN] % count);
	    groups->dimGroupN[dimN] = 1;
	    for (dN = dimN + dimStep; dN != lsDimN + dimStep; dN += dimStep) {
	       groups->nDimGroups[dN] = 1;
	       groups->firstDimCount[dN] = dimSizes[dN];
	       groups->lastDimCount[dN] = dimSizes[dN];
	       groups->dimGroupN[dN] = 1;
	    }
	    return TRUE;
	  }
	}
	count = (count + 1) / 2;
     }
     groups->nDimGroups[dimN] = dimSizes[dimN];
     groups->firstDimCount[dimN] = 1;
     groups->lastDimCount[dimN] = 1;
     groups->dimGroupN[dimN] = 1;
     nHypersSoFar *= dimSizes[dimN];
  }
  if (AttemptAllocations(1L,nScalarBuffers,1L,nHyperBuffers,
			 handles,nValueBytes)) return TRUE;
  /****************************************************************************
  * Not all of the buffers can be allocated even at only one value per buffer.
  ****************************************************************************/
  if (fatalFnc != NULL) (*fatalFnc)("Unable to allocate memory buffers.");
  return FALSE;
}

/******************************************************************************
* AttemptAllocations.
******************************************************************************/

static Logical AttemptAllocations (nScalarValues, nScalarBuffers, nHyperValues,
				   nHyperBuffers, handles, nValueBytes)
long nScalarValues;
int nScalarBuffers;
long nHyperValues;
int nHyperBuffers;
Byte **handles[];
size_t nValueBytes[];
{
  int bN, bNt; uLong maxBytes = 1, nBytes;
#if defined(mac) || defined(dos)
  void *ptr;
#endif
  /****************************************************************************
  * First allocate scalar buffers.
  ****************************************************************************/
  for (bN = 0; bN < nScalarBuffers; bN++) {
     nBytes = nScalarValues * nValueBytes[bN];
     if (nBytes > MAXnBYTESperALLOCATION)
       *handles[bN] = NULL;
     else
       *handles[bN] = cdf_AllocateMemory ((size_t) nBytes, NULL);
     if (*handles[bN] == NULL) {
       for (bNt = bN - 1; bNt >= 0; bNt--) cdf_FreeMemory (*handles[bNt],
						       NULL);
       return FALSE;
     }
     maxBytes = MAXIMUM(maxBytes,nBytes);
  }
  /****************************************************************************
  * Then allocate hyper buffers.
  ****************************************************************************/
  for (bN = nScalarBuffers; bN < nScalarBuffers + nHyperBuffers; bN++) {
     nBytes = nHyperValues * nValueBytes[bN];
     if (nBytes > MAXnBYTESperALLOCATION)
       *handles[bN] = NULL;
     else
       *handles[bN] = cdf_AllocateMemory ((size_t) nBytes, NULL);
     if (*handles[bN] == NULL) {
       for (bNt = bN - 1; bNt >= 0; bNt--) cdf_FreeMemory (*handles[bNt],
						       NULL);
       return FALSE;
     }
     maxBytes = MAXIMUM(maxBytes,nBytes);
  }
  /****************************************************************************
  * Allocate and then free a buffer twice the size of the largest buffer
  * allocated.  This will save some space on the heap for other uses.
  ****************************************************************************/
#if defined(mac) || defined(dos)
  ptr = cdf_AllocateMemory ((size_t) (2 * maxBytes), NULL);
  if (ptr == NULL) {
    for (bNt = nScalarBuffers + nHyperBuffers - 1; bNt >= 0; bNt--) {
       cdf_FreeMemory (*handles[bNt], NULL);
    }
    return FALSE;
  }
  cdf_FreeMemory (ptr, NULL);
#endif
  /****************************************************************************
  * All allocations succeeded, return TRUE.
  ****************************************************************************/
  return TRUE;
}

/******************************************************************************
* HyperFullRecord.
******************************************************************************/

Logical HyperFullRecord (groups, numDims)
struct GroupStruct *groups;
long numDims;
{
  int dimN;
  for (dimN = 0; dimN < numDims; dimN++) {
     if (groups->nDimGroups[dimN] > 1) return FALSE;
  }
  return TRUE;
}

/******************************************************************************
* HypersPerRecord.
******************************************************************************/

long HypersPerRecord (groups, numDims)
struct GroupStruct *groups;
long numDims;
{
  int dimN; long nHypers = 1;
  for (dimN = 0; dimN < numDims; dimN++) {
     nHypers *= groups->nDimGroups[dimN];
  }
  return nHypers;
}

/******************************************************************************
* HyperStartRecord.
******************************************************************************/

Logical HyperStartOfRecord (hyper, numDims)
struct HyperStruct *hyper;
long numDims;
{
  int dimN;
  for (dimN = 0; dimN < numDims; dimN++) {
     if (hyper->dimIndices[dimN] > 0) return FALSE;
  }
  return TRUE;
}

/******************************************************************************
* InitHyperParms.
******************************************************************************/

void InitHyperParms (hyper, groups, numDims, nHypers, nValues)
struct HyperStruct *hyper;
struct GroupStruct *groups;
long numDims;
long *nHypers;
long *nValues;
{
  int dimN;
  *nHypers = groups->nRecGroups;
  for (dimN = 0; dimN < numDims; dimN++) *nHypers *= groups->nDimGroups[dimN];
  hyper->recNumber = 0;
  hyper->recCount = groups->firstRecCount;
  hyper->recInterval = 1;
  for (dimN = 0; dimN < numDims; dimN++) {
     hyper->dimIndices[dimN] = 0;
     hyper->dimCounts[dimN] = groups->firstDimCount[dimN];
     hyper->dimIntervals[dimN] = 1;
  }
  *nValues = hyper->recCount;
  for (dimN = 0; dimN < numDims; dimN++) *nValues *= hyper->dimCounts[dimN];
  return;
}

/******************************************************************************
* IncrHyperParms.
******************************************************************************/

void IncrHyperParms (hyper, groups, numDims, rowMajor, nValues)
struct HyperStruct *hyper;
struct GroupStruct *groups;
long numDims;
Logical rowMajor;
long *nValues;
{
  int lsDimN;           /* Least significant dimension number. */
  int msDimN;           /* Most significant dimension number. */
  int dimStep;          /* How to increment dimension number from most
			   significant to least significant. */
  int dimN;
  /****************************************************************************
  * Setup for majority.
  ****************************************************************************/
  lsDimN = BOO(rowMajor,(int)(numDims-1),0);
  msDimN = BOO(rowMajor,0,(int)(numDims-1));
  dimStep = BOO(rowMajor,1,-1);
  /****************************************************************************
  * Increment indices.
  ****************************************************************************/
  for (dimN = lsDimN; dimN != msDimN - dimStep; dimN -= dimStep) {
     if (groups->dimGroupN[dimN] < groups->nDimGroups[dimN]) {
       groups->dimGroupN[dimN]++;
       if (groups->dimGroupN[dimN] < groups->nDimGroups[dimN]) {
	 hyper->dimIndices[dimN] += groups->firstDimCount[dimN];
	 hyper->dimCounts[dimN] = groups->firstDimCount[dimN];
       }
       else {
	 hyper->dimIndices[dimN] += groups->firstDimCount[dimN];
	 hyper->dimCounts[dimN] = groups->lastDimCount[dimN];
       }
       *nValues = hyper->recCount;
       for (dimN = 0; dimN < numDims; dimN++) {
	  *nValues *= hyper->dimCounts[dimN];
       }
       return;
     }
     else {
       groups->dimGroupN[dimN] = 1;
       hyper->dimIndices[dimN] = 0;
       hyper->dimCounts[dimN] = groups->firstDimCount[dimN];
     }
  }
  /****************************************************************************
  * Increment record.
  ****************************************************************************/
  groups->recGroupN++;
  hyper->recNumber += groups->firstRecCount;
  hyper->recCount = BOO(groups->recGroupN < groups->nRecGroups,
			groups->firstRecCount,groups->lastRecCount);
  *nValues = hyper->recCount;
  for (dimN = 0; dimN < numDims; dimN++) *nValues *= hyper->dimCounts[dimN];
  return;
}

/******************************************************************************
* WriteOutPct.
******************************************************************************/

void WriteOutPct (pct)
int pct;
{
  char text[8+1];
  strcpyX (text, "\b\b\b\b", 8);
  if (pct < 10) strcatX (text, ".", 8);
  if (pct < 100) strcatX (text, ".", 8);
  sprintf (EofS(text), "%d", pct);
  strcatX (text, "%", 8);
  WriteOut (stdout, text);
  return;
}

/******************************************************************************
* SwitchMajority.
******************************************************************************/

Logical SwitchMajority (buffer, rowMajor, numDims, dimSizes, recCount,
			nValueBytes)
Byte *buffer;
Logical rowMajor;
long numDims;
long dimSizes[];
long recCount;
size_t nValueBytes;
{
  int dimN; long nRecordBytes, recX; Byte *offset; void *tBuffer;
  /****************************************************************************
  * If less than two dimensions no switching needs to be done.
  ****************************************************************************/
  if (numDims < 2) return TRUE;
  /****************************************************************************
  * Calculate number of bytes per record.
  ****************************************************************************/
  nRecordBytes = nValueBytes;
  for (dimN = 0; dimN < numDims; dimN++) nRecordBytes *= dimSizes[dimN];
  /****************************************************************************
  * Attempt to allocate a temporary buffer for one record.
  ****************************************************************************/
#if defined(dos)
  if (TOObigIBMpc(nRecordBytes))
    tBuffer = NULL;
  else
#endif
    tBuffer = cdf_AllocateMemory ((size_t) nRecordBytes, NULL);
  if (tBuffer == NULL) return FALSE;
  /****************************************************************************
  * Switch majority.
  ****************************************************************************/
  for (recX = 0, offset = buffer; recX < recCount;
       recX++, offset += (size_t) nRecordBytes) {
    if (rowMajor)
      ROWtoCOL (offset, tBuffer, numDims, dimSizes, (long) nValueBytes);
    else
      COLtoROW (offset, tBuffer, numDims, dimSizes, (long) nValueBytes);
    memmove (offset, tBuffer, (size_t) nRecordBytes);
  }
  /****************************************************************************
  * Free temporary buffer and return success.
  ****************************************************************************/
  cdf_FreeMemory (tBuffer, FatalError);
  return TRUE;
}

/******************************************************************************
* CompressedCDF.
******************************************************************************/

Logical CompressedCDF (CDFpath)
char *CDFpath;
{
  CDFstatus status; long cType, cParms[CDF_MAX_PARMS];
  OFF_T cFileSize, uFileSize;
  status = CDFlib (GET_, CDF_INFO_, CDFpath, &cType, cParms,
                                    &cFileSize, &uFileSize,
                   NULL_);
  if (StatusBAD(status)) return FALSE;
  return ((Logical) (cType != NO_COMPRESSION));
}

/******************************************************************************
* SameCompressions.
******************************************************************************/

Logical SameCompressions (cType1, cParms1, cType2, cParms2)
long cType1;
long cParms1[CDF_MAX_PARMS];
long cType2;
long cParms2[CDF_MAX_PARMS];
{
  int parmN, pCount;
  /****************************************************************************
  * Compare compression types.
  ****************************************************************************/
  if (cType1 != cType2) return FALSE;
  /****************************************************************************
  * If same types, compare compression parameters (if any).
  ****************************************************************************/
  pCount = CompressionParmsCount(cType1);
  for (parmN = 0; parmN < pCount; parmN++) {
     if (cParms1[parmN] != cParms2[parmN]) return FALSE;
  }
  /****************************************************************************
  * Compressions must be the same.
  ****************************************************************************/
  return TRUE;
}

/******************************************************************************
* SameSparsenesses.
******************************************************************************/

Logical SameSparsenesses (sRecordsType1, sArraysType1, sArraysParms1,
			  sRecordsType2, sArraysType2, sArraysParms2)
long sRecordsType1;
long sArraysType1;
long sArraysParms1[CDF_MAX_PARMS];
long sRecordsType2;
long sArraysType2;
long sArraysParms2[CDF_MAX_PARMS];
{
  int parmN, pCount;
  /****************************************************************************
  * Compare sparse records.
  ****************************************************************************/
  if (sRecordsType1 != sRecordsType2) return FALSE;
  /****************************************************************************
  * Compare sparse array types.
  ****************************************************************************/
  if (sArraysType1 != sArraysType2) return FALSE;
  /****************************************************************************
  * If same types, compare sparse array parameters (if any).
  ****************************************************************************/
  pCount = SparsenessParmsCount(sArraysType1);
  for (parmN = 0; parmN < pCount; parmN++) {
     if (sArraysParms1[parmN] != sArraysParms2[parmN]) return FALSE;
  }
  /****************************************************************************
  * Sparsenesses must be the same.
  ****************************************************************************/
  return TRUE;
}

/******************************************************************************
* WhichFormat.
******************************************************************************/

long WhichFormat (token)
char *token;
{
  if (!strncmpIgCase(token,"SINGLE",6)) return SINGLE_FILE;
  if (!strncmpIgCase(token,"MULTI",5)) return MULTI_FILE;
  return -1;
}

/******************************************************************************
* WhichMajority.
******************************************************************************/

long WhichMajority (token)
char *token;
{
  if (!strncmpIgCase(token,"ROW",3)) return ROW_MAJOR;
  if (!strncmpIgCase(token,"COL",3) ||
      !strncmp(token,"COLUMN",6)) return COLUMN_MAJOR;
  return -1;
}

/******************************************************************************
* WhichRecSparseness.
******************************************************************************/

long WhichRecSparseness (token)
char *token;
{
  if (!strncmpIgCase(token,"None",4)) return NO_SPARSERECORDS;
  if (!strncmpIgCase(token,"sRecords.PAD",12)) return PAD_SPARSERECORDS;
  if (!strncmpIgCase(token,"sRecords.PREV",13)) return PREV_SPARSERECORDS;
  return -1;
}

/******************************************************************************
* WhichCompression
******************************************************************************/

long WhichCompression (token, compression, cParms)
char *token;
long *compression;
long cParms[CDF_MAX_PARMS];
{
  *compression = -1;
  cParms[0] = 0;
  if (!strncmpIgCase(token,"None",4)) *compression = NO_COMPRESSION;
  if (!strncmpIgCase(token,"RLE",3)) *compression = RLE_COMPRESSION;
  if (!strncmpIgCase(token,"HUFF",4))  *compression = HUFF_COMPRESSION;
  if (!strncmpIgCase(token,"AHUFF",5)) *compression = AHUFF_COMPRESSION;
  if (!strncmpIgCase(token,"GZIP",4)) {
    int clevel;
    if (sscanf(token,"GZIP.%1d",&clevel) == 1)
        if (INCLUSIVE(1,clevel,9)) {
	  *compression =  GZIP_COMPRESSION;
	  cParms[0] = (long) clevel;
	}
  }
/*  if (!strncmpIgCase(token,"ZLIB",4)) *compression = ZLIB_COMPRESSION; */
  return (long) *compression;
}

/******************************************************************************
* WhichEncoding.  `MIPSEL' (DECstation) and `MIPSEB' (SGi) are valid synonyms
*                 in order to remain compatible with previous releases of CDF.
******************************************************************************/

long WhichEncoding (token)
char *token;
{
  if (!strncmpIgCase(token,"HOST",4)) return HOST_ENCODING;
  if (!strncmpIgCase(token,"NETWORK",6)) return NETWORK_ENCODING;
  if (!strncmpIgCase(token,"VAX",3)) return VAX_ENCODING;
  if (!strncmpIgCase(token,"SUN",3)) return SUN_ENCODING;
  if (!strncmpIgCase(token,"SGi",3) ||
      !strncmpIgCase(token,"MIPSEB",6)) return SGi_ENCODING;
  if (!strncmpIgCase(token,"DECSTATION",10) ||
      !strncmpIgCase(token,"MIPSEL",6)) return DECSTATION_ENCODING;
  if (!strncmpIgCase(token,"IBMRS",5)) return IBMRS_ENCODING;
  if (!strncmpIgCase(token,"IBMPC",5)) return IBMPC_ENCODING;
  if (!strncmpIgCase(token,"HP",2)) return HP_ENCODING;
  if (!strncmpIgCase(token,"NeXT",4)) return NeXT_ENCODING;
  if (!strncmpIgCase(token,"ALPHAOSF1",9)) return ALPHAOSF1_ENCODING;
  if (!strncmpIgCase(token,"ALPHAVMSd",9)) return ALPHAVMSd_ENCODING;
  if (!strncmpIgCase(token,"ALPHAVMSg",9)) return ALPHAVMSg_ENCODING;
  if (!strncmpIgCase(token,"ALPHAVMSi",9)) return ALPHAVMSi_ENCODING;
  if (!strncmpIgCase(token,"MAC",3)) return MAC_ENCODING;
  return -1;
}

/******************************************************************************
* WhichDataType.
******************************************************************************/

long WhichDataType (token)
char *token;
{
  if (!strncmpIgCase(token,"BYTE",4)) return CDF_BYTE;
  if (!strncmpIgCase(token,"INT1",4)) return CDF_INT1;
  if (!strncmpIgCase(token,"UINT1",5)) return CDF_UINT1;
  if (!strncmpIgCase(token,"INT2",4)) return CDF_INT2;
  if (!strncmpIgCase(token,"UINT2",5)) return CDF_UINT2;
  if (!strncmpIgCase(token,"INT4",4)) return CDF_INT4;
  if (!strncmpIgCase(token,"UINT4",5)) return CDF_UINT4;
  if (!strncmpIgCase(token,"REAL4",5)) return CDF_REAL4;
  if (!strncmpIgCase(token,"FLOAT",5)) return CDF_FLOAT;
  if (!strncmpIgCase(token,"REAL8",5)) return CDF_REAL8;
  if (!strncmpIgCase(token,"DOUBLE",6)) return CDF_DOUBLE;
  if (!strncmpIgCase(token,"CHAR",4)) return CDF_CHAR;
  if (!strncmpIgCase(token,"UCHAR",5)) return CDF_UCHAR;
  if (!strncmpIgCase(token,"EPOCH",5)) return CDF_EPOCH;
  if (!strncmpIgCase(token,"EPOCH16",7)) return CDF_EPOCH16;
  if (!strncmpIgCase(token,"CDF_BYTE",8)) return CDF_BYTE;
  if (!strncmpIgCase(token,"CDF_INT1",8)) return CDF_INT1;
  if (!strncmpIgCase(token,"CDF_UINT1",9)) return CDF_UINT1;
  if (!strncmpIgCase(token,"CDF_INT2",8)) return CDF_INT2;
  if (!strncmpIgCase(token,"CDF_UINT2",9)) return CDF_UINT2;
  if (!strncmpIgCase(token,"CDF_INT4",8)) return CDF_INT4;
  if (!strncmpIgCase(token,"CDF_UINT4",9)) return CDF_UINT4;
  if (!strncmpIgCase(token,"CDF_REAL4",9)) return CDF_REAL4;
  if (!strncmpIgCase(token,"CDF_FLOAT",9)) return CDF_FLOAT;
  if (!strncmpIgCase(token,"CDF_REAL8",9)) return CDF_REAL8;
  if (!strncmpIgCase(token,"CDF_DOUBLE",10)) return CDF_DOUBLE;
  if (!strncmpIgCase(token,"CDF_CHAR",8)) return CDF_CHAR;
  if (!strncmpIgCase(token,"CDF_UCHAR",9)) return CDF_UCHAR;
  if (!strncmpIgCase(token,"CDF_EPOCH",9)) return CDF_EPOCH;
  if (!strncmpIgCase(token,"CDF_EPOCH16",11)) return CDF_EPOCH16;
  return -1;
}

/******************************************************************************
* EncodingToken.
*   Returns address of character string for encoding.
******************************************************************************/

char *EncodingToken (encoding)
long encoding;
{
  switch (encoding) {
    case NETWORK_ENCODING: return "NETWORK";
    case VAX_ENCODING: return "VAX";
    case SUN_ENCODING: return "SUN";
    case DECSTATION_ENCODING: return "DECSTATION";
    case SGi_ENCODING: return "SGi";
    case IBMRS_ENCODING: return "IBMRS";
    case IBMPC_ENCODING: return "IBMPC";
    case HP_ENCODING: return "HP";
    case NeXT_ENCODING: return "NeXT";
    case ALPHAOSF1_ENCODING: return "ALPHAOSF1";
    case ALPHAVMSd_ENCODING: return "ALPHAVMSd";
    case ALPHAVMSg_ENCODING: return "ALPHAVMSg";
    case ALPHAVMSi_ENCODING: return "ALPHAVMSi";
    case MAC_ENCODING: return "MAC";
    default: return "?";
  }
}

/******************************************************************************
* MajorityToken.
*   Returns address of character string for majority.
******************************************************************************/

char *MajorityToken (majority)
long majority;
{
  switch (majority) {
    case ROW_MAJOR: return "ROW";
    case COLUMN_MAJOR: return "COLUMN";
    default: return "?";
  }
}

/******************************************************************************
* FormatToken.
*   Returns address of character string for format.
******************************************************************************/

char *FormatToken (format)
long format;
{
  switch (format) {
    case SINGLE_FILE: return "SINGLE";
    case MULTI_FILE: return "MULTI";
    default: return "?";
  }
}

/******************************************************************************
* ScopeToken.
*   Returns address of character string for scope.
******************************************************************************/

char *ScopeToken (scope)
long scope;
{
  switch (scope) {
    case GLOBAL_SCOPE: return "GLOBAL";
    case VARIABLE_SCOPE: return "VARIABLE";
    default: return "?";
  }
}

/******************************************************************************
* VarianceToken.
*   Returns address of character string for variance.
******************************************************************************/

char *VarianceToken (variance)
long variance;
{
  switch (variance) {
    case VARY: return "VARY";
    case NOVARY: return "NOVARY";
    default: return "?";
  }
}

/******************************************************************************
* TFvarianceToken.
*   Returns address of character string for variance (T/F).
******************************************************************************/

char *TFvarianceToken (variance)
long variance;
{
  switch (variance) {
    case VARY: return "T";
    case NOVARY: return "F";
    default: return "?";
  }
}

/******************************************************************************
* DataTypeToken.
*   Returns address of character string for data type.
******************************************************************************/

char *DataTypeToken (dataType)
long dataType;
{
  switch (dataType) {
    case CDF_BYTE: return "BYTE";
    case CDF_INT1: return "INT1";
    case CDF_INT2: return "INT2";
    case CDF_INT4: return "INT4";
    case CDF_UINT1: return "UINT1";
    case CDF_UINT2: return "UINT2";
    case CDF_UINT4: return "UINT4";
    case CDF_REAL4: return "REAL4";
    case CDF_REAL8: return "REAL8";
    case CDF_FLOAT: return "FLOAT";
    case CDF_DOUBLE: return "DOUBLE";
    case CDF_EPOCH: return "EPOCH";
    case CDF_EPOCH16: return "EPOCH16";
    case CDF_CHAR: return "CHAR";
    case CDF_UCHAR: return "UCHAR";
    default: return "?";
  }
}

/******************************************************************************
* CompressionToken.
*   Returns address of character string for compression type/parameters.
******************************************************************************/

char *CompressionToken (cType, cParms)
long cType;
long cParms[CDF_MAX_PARMS];
{
  switch (cType) {
    case NO_COMPRESSION:
      return "None";
    case RLE_COMPRESSION:
      switch (cParms[0]) {
	case RLE_OF_ZEROs: return "RLE.0";
	default: return "RLE.?";
      }
    case HUFF_COMPRESSION:
      switch (cParms[0]) {
	case OPTIMAL_ENCODING_TREES: return "HUFF.0";
	default: return "HUFF.?";
      }
    case AHUFF_COMPRESSION:
      switch (cParms[0]) {
	case OPTIMAL_ENCODING_TREES: return "AHUFF.0";
	default: return "AHUFF.?";
      }
    case GZIP_COMPRESSION: {
      static char token[MAX_GZIP_TOKEN_LEN+1];
      sprintf (token, "GZIP.%d", (int) cParms[0]);
      return token;
    }
/*
    case ZLIB_COMPRESSION:
      switch (cParms[0]) {
        case RLE_OF_ZEROs: return "ZLIB.0";
        default: return "ZLIB.?";
      }
*/
    default: return "?";
  }
}

/******************************************************************************
* SparsenessToken.
*   Returns address of character string for sparseness types/parameters.
* Both record and array sparseness is encoded in the same string.
******************************************************************************/

char *SparsenessToken (sRecordsType, sArraysType, sArraysParms)
long sRecordsType;
long sArraysType;
long sArraysParms[CDF_MAX_PARMS];
{
  if (sArraysParms[0] == 0) { };    /* Quiets some compilers. */
  switch (sRecordsType) {
    case NO_SPARSERECORDS:
      switch (sArraysType) {
	case NO_SPARSEARRAYS:
	  return "None";
	default:
	  return "?";
      }
    case PAD_SPARSERECORDS:
      switch (sArraysType) {
	case NO_SPARSEARRAYS:
	  return "sRecords.PAD";
	default:
	  return "?";
      }
    case PREV_SPARSERECORDS:
      switch (sArraysType) {
	case NO_SPARSEARRAYS:
	  return "sRecords.PREV";
	default:
	  return "?";
      }
    default:
      return "?";
  }
}

/******************************************************************************
* PickDelimiter.
*    Exclamation mark (`!') isn't used because it starts a comment in a
* skeleton table.  Period (`.') isn't used because it ends a list of
* attribute entries (in a skeleton table).  Left bracket (`[') isn't used
* because it starts a set of NRV indices (in a skeleton table).
******************************************************************************/

char PickDelimiter (string, N)
char *string;
size_t N;
{
  static char choices[] = "\"'`^~:-,;|+=@#$%&*()/?<>{}]\\";
  int i;
  for (i = 0; choices[i] != NUL; i++) {
     if (strnchr(string,choices[i],N) == NULL) return choices[i];
  }
  return NUL;   /* Very bad, use character stuffing if this is a problem. */
}

/******************************************************************************
* strnchr.
******************************************************************************/

static char *strnchr (string, chr, n)
char *string;
int chr;
int n;
{
  int i;
  for (i = 0; i < n; i++) if (string[i] == chr) return &string[i];
  return NULL;
}

/******************************************************************************
* sleep.  Only for Microsoft C.
******************************************************************************/

#if defined(MICROSOFTC)
void sleep (seconds)
uInt seconds;
{
time_t start_time;
time_t current_time;

time (&start_time);
time (&current_time);

while (current_time - start_time < seconds) time (&current_time);

return;
}
#endif

/******************************************************************************
* strstrIgCase.
******************************************************************************/

char *strstrIgCase (string, substring)
char *string;
char *substring;
{
  int stringL = strlen(string);
  int substringL = strlen(substring);
  int i, j;
  if (stringL == 0 || substringL == 0) return NULL;
  if (stringL < substringL) return NULL;
  for (i = 0; i < stringL; i++) {
     for (j = 0; j < substringL; j++) {
	if (MakeLower(substring[j]) != MakeLower(string[i+j])) break;
     }
     if (j == substringL) return &string[i];
  }
  return NULL;
}

/******************************************************************************
* strcmpIgCase.
******************************************************************************/

int strcmpIgCase (string1, string2)
char *string1;
char *string2;
{
  int string1L = strlen(string1);
  int string2L = strlen(string2);
  int maxL = MAXIMUM(string1L,string2L);
  int i;
  for (i = 0; i < maxL; i++) {
     if (i > string1L || i > string2L) break;
     if (MakeLower(string1[i]) != MakeLower(string2[i])) {
       return ((int) (string1[i] - string2[i]));
     }
  }
  if (string1L > string2L) return 1;
  else if (string1L < string2L) return -1;
  return 0;
}

/******************************************************************************
* strncmpIgCase... Assuming count is the length from one of passed strings.
******************************************************************************/

int strncmpIgCase (string1, string2, count)
char *string1;
char *string2;
size_t count;
{
  int string1L = (int) strlen(string1);
  int string2L = (int) strlen(string2);
  int maxL, i;
  maxL = MAXIMUM(string1L,string2L);
  maxL = MINIMUM(maxL,(int)count);
  for (i = 0; i < maxL; i++) {
     if (i > string1L || i > string2L) break;
     if (MakeLower(string1[i]) != MakeLower(string2[i])) {
       return ((int) (string1[i] - string2[i]));
     }
  }
  if (string1L > string2L) return 1;
  else if (string1L < string2L) return -1;
  return 0;
}

/******************************************************************************
* strncmpIgCasePattern.
******************************************************************************/

int strncmpIgCasePattern (string1, string2, count)
char *string1;
char *string2;
size_t count;
{
  int string1L = (int) strlen(string1);
  int string2L = (int) strlen(string2);
  int maxL, i;
  maxL = MAXIMUM(string1L,string2L);
  maxL = MINIMUM(maxL,(int)count);
  for (i = 0; i < maxL; i++) {
    if (MakeLower(string1[i]) != MakeLower(string2[i])) {
      return ((int) (string1[i] - string2[i]));
    }
  }
  return 0;
}

/******************************************************************************
* strchrIgCase.
******************************************************************************/

char *strchrIgCase (str, chr)
char *str;
int chr;
{
  int strL = strlen(str);
  int i;
  if (strL == 0) return NULL;
  for (i = 0; i < strL; i++) {
     if (MakeLower(chr) == MakeLower(str[i])) return &str[i];
  }
  return NULL;
}

/******************************************************************************
* WriteEntryValue.
******************************************************************************/

void WriteEntryValue (fp, dataType, numElements, value, ccc, MaXcc)
FILE *fp;
long dataType;
long numElements;
void *value;
int ccc;                /* Current Cursor Column (at start). */
int MaXcc;              /* Maximum Cursor Column at which to output. */
{
  if (STRINGdataType(dataType))
    WriteStringValue (fp, numElements, value, ccc, MaXcc);
  else {
    int i, elemN, n, cccBase = ccc;
    char evalue[80+1];
    Logical newLine = TRUE;
    for (elemN = 0; elemN < numElements; elemN++) {
       n = EncodeValue (dataType, (Byte *) value + elemN*CDFelemSize(dataType),
			evalue, EPOCH0_STYLE);
       if (ccc + (newLine ? 0 : 1) + n +
	   (elemN != numElements - 1 ? 1 : 0) > MaXcc) {
	 WriteOut (fp, "\n");
	 for (i = 0; i < cccBase; i++) WriteOut (fp, " ");
	 ccc = cccBase;
	 newLine = TRUE;
       }
       if (!newLine) ccc += WriteOut (fp, " ");
       ccc += WriteOut (fp, evalue);
       if (elemN != numElements - 1) ccc += WriteOut (fp, ",");
       newLine = FALSE;
    }
  }
  return;
}

/******************************************************************************
* WriteStringValue.
*    Note that control characters may exist in the string being written (even
* a NUL character).  Control characters are to be replaced with `.'.  For this
* reason `memmove' is used to move around strings/substrings.
******************************************************************************/

void WriteStringValue (fp, numChars, value, ccc, MaXcc)
FILE *fp;
long numChars;
void *value;
int ccc;                /* Current Cursor Column (at start). */
int MaXcc;              /* Maximum Cursor Column at which to output. */
{
  int cccBase = ccc, i;
  char *string = (char *) value;
  char delim;
  size_t textL = MaXcc + 1;             /* MaXcc + 1. */
  char *text = (char *) cdf_AllocateMemory ((size_t) (textL + 1),
					FatalError);

  if (ccc + 1 + numChars + 1 - 1 <= MaXcc) {
    delim = PickDelimiter (string, (size_t) numChars);
    text[0] = delim;
    memmove (&text[1], string, (size_t) numChars);
    text[(int)numChars+1] = delim;
    text[(int)numChars+2] = NUL;
    for (i = 1; i <= numChars; i++) {
       if (!Printable(text[i])) text[i] = '.';
    }
    WriteOut (fp, text);
  }
  else {
    int sss = MaXcc - ccc - 4 + 1;      /* 4 for starting `"' and ending `" -'
					   (assuming `"' is the delimeter). */
    char *substring = (char *) cdf_AllocateMemory ((size_t) (sss + 1),
					       FatalError);
    int x = 0, len;
    while (x + sss < numChars) {
      if (string[x+sss] == ' ') /* First character next line will be a blank.*/
	len = sss;
      else {
	if (string[x+sss-1] == ' ') /* Last character on line is a blank. */
	  len = sss;
	else {                 /* Look for a blank at which to break line. */
	  for (i = x + sss - 2; i > x; i--) if (string[i] == ' ') break;
	  if (i > x)
	    len = i - x + 1;
	  else
	    len = sss;
	}
      }
      memmove (substring, &string[x], len);
      substring[len] = NUL;
      for (i = 0; i < len; i++) {
	 if (!Printable(substring[i])) substring[i] = '.';
      }
      delim = PickDelimiter (substring, strlen(substring));
      sprintf (text, "%c%s%c -", delim, substring, delim);
      WriteOut (fp, text);
      WriteOut (fp, "\n");
      for (i = 0; i < cccBase; i++) WriteOut (fp, " ");
      x += len;
    }
    len = (size_t) (numChars - x);
    memmove (substring, &string[x], len);
    substring[len] = NUL;
    for (i = 0; i < len; i++) {
       if (!Printable(substring[i])) substring[i] = '.';
    }
    delim = PickDelimiter (substring, strlen(substring));
    sprintf (text, "%c%s%c", delim, substring, delim);
    WriteOut (fp, text);
    cdf_FreeMemory (substring, FatalError);
  }
  cdf_FreeMemory (text, FatalError);
  return;
}

/******************************************************************************
* ConvertDataType.
******************************************************************************/

void ConvertDataType (fromDataType, fromNumElems, fromPtr, toDataType,
		      toNumElems, toPtr)
long fromDataType;
long fromNumElems;              /* Only used if character data type. */
void *fromPtr;
long toDataType;
long toNumElems;                /* Only used if character data type. */
void *toPtr;
{
  switch (fromDataType) {
    /**************************************************************************
    * 1-byte signed integer to...
    **************************************************************************/
    case CDF_BYTE:
    case CDF_INT1:
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1:
	  *((sChar *) toPtr) = (sChar) *((sChar *) fromPtr);
	  break;
	case CDF_UINT1:
	  *((uChar *) toPtr) = (uChar) *((sChar *) fromPtr);
	  break;
	case CDF_INT2:
	  *((Int16 *) toPtr) = (Int16) *((sChar *) fromPtr);
	  break;
	case CDF_UINT2:
	  *((uInt16 *) toPtr) = (uInt16) *((sChar *) fromPtr);
	  break;
	case CDF_INT4:
	  *((Int32 *) toPtr) = (Int32) *((sChar *) fromPtr);
	  break;
	case CDF_UINT4:
	  *((uInt32 *) toPtr) = (uInt32) *((sChar *) fromPtr);
	  break;
	case CDF_FLOAT:
	case CDF_REAL4:
	  *((float *) toPtr) = (float) *((sChar *) fromPtr);
	  break;
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8:
	  *((double *) toPtr) = (double) *((sChar *) fromPtr);
	  break;
        case CDF_EPOCH16:
          *((double *) toPtr) = (double) *((sChar *) fromPtr);
	  *(((double *) toPtr) + 1) = (double) *(((sChar *) fromPtr) + 1);
          break;
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i, len;
	  char tmp[MAX_nonSTRING_VALUE_LEN+1];
	  sprintf (tmp, "%d", *((sChar *) fromPtr));
	  for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
	  }
	  break;
	}
      }
      break;
    /**************************************************************************
    * 1-byte unsigned integer to...
    **************************************************************************/
    case CDF_UINT1:
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1:
	  *((sChar *) toPtr) = (sChar) *((uChar *) fromPtr);
	  break;
	case CDF_UINT1:
	  *((uChar *) toPtr) = (uChar) *((uChar *) fromPtr);
	  break;
	case CDF_INT2:
	  *((Int16 *) toPtr) = (Int16) *((uChar *) fromPtr);
	  break;
	case CDF_UINT2:
	  *((uInt16 *) toPtr) = (uInt16) *((uChar *) fromPtr);
	  break;
	case CDF_INT4:
	  *((Int32 *) toPtr) = (Int32) *((uChar *) fromPtr);
	  break;
	case CDF_UINT4:
	  *((uInt32 *) toPtr) = (uInt32) *((uChar *) fromPtr);
	  break;
	case CDF_FLOAT:
	case CDF_REAL4:
	  *((float *) toPtr) = (float) *((uChar *) fromPtr);
	  break;
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8:
	  *((double *) toPtr) = (double) *((uChar *) fromPtr);
	  break;
        case CDF_EPOCH16:
          *((double *) toPtr) = (double) *((uChar *) fromPtr);
	  *(((double *) toPtr) + 1) = (double) *(((uChar *) fromPtr) + 1);
          break;
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i, len;
	  char tmp[MAX_nonSTRING_VALUE_LEN+1];
	  sprintf (tmp, "%u", *((uChar *) fromPtr));
	  for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
	  }
	  break;
	}
      }
      break;
    /**************************************************************************
    * 2-byte signed integer to...
    **************************************************************************/
    case CDF_INT2:
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1:
	  *((sChar *) toPtr) = (sChar) *((Int16 *) fromPtr);
	  break;
	case CDF_UINT1:
	  *((uChar *) toPtr) = (uChar) *((Int16 *) fromPtr);
	  break;
	case CDF_INT2:
	  *((Int16 *) toPtr) = (Int16) *((Int16 *) fromPtr);
	  break;
	case CDF_UINT2:
	  *((uInt16 *) toPtr) = (uInt16) *((Int16 *) fromPtr);
	  break;
	case CDF_INT4:
	  *((Int32 *) toPtr) = (Int32) *((Int16 *) fromPtr);
	  break;
	case CDF_UINT4:
	  *((uInt32 *) toPtr) = (uInt32) *((Int16 *) fromPtr);
	  break;
	case CDF_FLOAT:
	case CDF_REAL4:
	  *((float *) toPtr) = (float) *((Int16 *) fromPtr);
	  break;
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8:
	  *((double *) toPtr) = (double) *((Int16 *) fromPtr);
	  break;
        case CDF_EPOCH16:
          *((double *) toPtr) = (double) *((Int16 *) fromPtr);
	  *(((double *) toPtr) + 1) = (double) *(((Int16 *) fromPtr) + 1);
          break;
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i, len;
	  char tmp[MAX_nonSTRING_VALUE_LEN+1];
	  sprintf (tmp, "%d", (int) *((Int16 *) fromPtr));
	  for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
	  }
	  break;
	}
      }
      break;
    /**************************************************************************
    * 2-byte unsigned integer to...
    **************************************************************************/
    case CDF_UINT2:
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1:
	  *((sChar *) toPtr) = (sChar) *((uInt16 *) fromPtr);
	  break;
	case CDF_UINT1:
	  *((uChar *) toPtr) = (uChar) *((uInt16 *) fromPtr);
	  break;
	case CDF_INT2:
	  *((Int16 *) toPtr) = (Int16) *((uInt16 *) fromPtr);
	  break;
	case CDF_UINT2:
	  *((uInt16 *) toPtr) = (uInt16) *((uInt16 *) fromPtr);
	  break;
	case CDF_INT4:
	  *((Int32 *) toPtr) = (Int32) *((uInt16 *) fromPtr);
	  break;
	case CDF_UINT4:
	  *((uInt32 *) toPtr) = (uInt32) *((uInt16 *) fromPtr);
	  break;
	case CDF_FLOAT:
	case CDF_REAL4:
	  *((float *) toPtr) = (float) *((uInt16 *) fromPtr);
	  break;
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8:
	  *((double *) toPtr) = (double) *((uInt16 *) fromPtr);
	  break;
        case CDF_EPOCH16:
          *((double *) toPtr) = (double) *((uInt16 *) fromPtr);
	  *(((double *) toPtr) + 1) = (double) *(((uInt16 *) fromPtr) + 1);
          break;
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i, len;
	  char tmp[MAX_nonSTRING_VALUE_LEN+1];
	  sprintf (tmp, "%u", (int) *((uInt16 *) fromPtr));
	  for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
	  }
	  break;
	}
      }
      break;
    /**************************************************************************
    * 4-byte signed integer to...
    **************************************************************************/
    case CDF_INT4:
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1:
	  *((sChar *) toPtr) = (sChar) *((Int32 *) fromPtr);
	  break;
	case CDF_UINT1:
	  *((uChar *) toPtr) = (uChar) *((Int32 *) fromPtr);
	  break;
	case CDF_INT2:
	  *((Int16 *) toPtr) = (Int16) *((Int32 *) fromPtr);
	  break;
	case CDF_UINT2:
	  *((uInt16 *) toPtr) = (uInt16) *((Int32 *) fromPtr);
	  break;
	case CDF_INT4:
	  *((Int32 *) toPtr) = (Int32) *((Int32 *) fromPtr);
	  break;
	case CDF_UINT4:
	  *((uInt32 *) toPtr) = (uInt32) *((Int32 *) fromPtr);
	  break;
	case CDF_FLOAT:
	case CDF_REAL4:
	  *((float *) toPtr) = (float) *((Int32 *) fromPtr);
	  break;
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8:
	  *((double *) toPtr) = (double) *((Int32 *) fromPtr);
	  break;
        case CDF_EPOCH16:
          *((double *) toPtr) = (double) *((Int32 *) fromPtr);
	  *(((double *) toPtr) + 1) = (double) *(((Int32 *) fromPtr) + 1);
          break;
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i, len;
	  char tmp[MAX_nonSTRING_VALUE_LEN+1];
	  sprintf (tmp, "%ld", (long) *((Int32 *) fromPtr));
	  for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
	  }
	  break;
	}
      }
      break;
    /**************************************************************************
    * 4-byte unsigned integer to...
    **************************************************************************/
    case CDF_UINT4:
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1:
	  *((sChar *) toPtr) = (sChar) *((uInt32 *) fromPtr);
	  break;
	case CDF_UINT1:
	  *((uChar *) toPtr) = (uChar) *((uInt32 *) fromPtr);
	  break;
	case CDF_INT2:
	  *((Int16 *) toPtr) = (Int16) *((uInt32 *) fromPtr);
	  break;
	case CDF_UINT2:
	  *((uInt16 *) toPtr) = (uInt16) *((uInt32 *) fromPtr);
	  break;
	case CDF_INT4:
	  *((Int32 *) toPtr) = (Int32) *((uInt32 *) fromPtr);
	  break;
	case CDF_UINT4:
	  *((uInt32 *) toPtr) = (uInt32) *((uInt32 *) fromPtr);
	  break;
	case CDF_FLOAT:
	case CDF_REAL4:
	  *((float *) toPtr) = (float) *((uInt32 *) fromPtr);
	  break;
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8:
	  *((double *) toPtr) = (double) *((uInt32 *) fromPtr);
	  break;
        case CDF_EPOCH16:
          *((double *) toPtr) = (double) *((uInt32 *) fromPtr);
	  *(((double *) toPtr) + 1) = (double) *(((uInt32 *) fromPtr) + 1);
          break;
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i, len;
	  char tmp[MAX_nonSTRING_VALUE_LEN+1];
	  sprintf (tmp, "%lu", (uLong) *((uInt32 *) fromPtr));
	  for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
	  }
	  break;
	}
      }
      break;
    /**************************************************************************
    * 4-byte floating-point to...
    **************************************************************************/
    case CDF_FLOAT:
    case CDF_REAL4:
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1:
	  *((sChar *) toPtr) = (sChar) *((float *) fromPtr);
	  break;
	case CDF_UINT1:
	  *((uChar *) toPtr) = (uChar) *((float *) fromPtr);
	  break;
	case CDF_INT2:
	  *((Int16 *) toPtr) = (Int16) *((float *) fromPtr);
	  break;
	case CDF_UINT2:
	  *((uInt16 *) toPtr) = (uInt16) *((float *) fromPtr);
	  break;
	case CDF_INT4:
	  *((Int32 *) toPtr) = (Int32) *((float *) fromPtr);
	  break;
	case CDF_UINT4:
	  *((uInt32 *) toPtr) = (uInt32) *((float *) fromPtr);
	  break;
	case CDF_FLOAT:
	case CDF_REAL4:
	  *((float *) toPtr) = (float) *((float *) fromPtr);
	  break;
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8:
	  *((double *) toPtr) = (double) *((float *) fromPtr);
	  break;
        case CDF_EPOCH16:
          *((double *) toPtr) = (double) *((float *) fromPtr);
	  *(((double *) toPtr) + 1) = (double) *(((float *) fromPtr) + 1);
          break;
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i, len;
	  char tmp[MAX_nonSTRING_VALUE_LEN+1];
	  sprintf (tmp, "%f", *((float *) fromPtr));
	  for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
	  }
	  break;
	}
      }
      break;
    /**************************************************************************
    * 8-byte floating-point to...
    **************************************************************************/
    case CDF_EPOCH:
    case CDF_DOUBLE:
    case CDF_REAL8:
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1:
	  *((sChar *) toPtr) = (sChar) *((double *) fromPtr);
	  break;
	case CDF_UINT1:
	  *((uChar *) toPtr) = (uChar) *((double *) fromPtr);
	  break;
	case CDF_INT2:
	  *((Int16 *) toPtr) = (Int16) *((double *) fromPtr);
	  break;
	case CDF_UINT2:
	  *((uInt16 *) toPtr) = (uInt16) *((double *) fromPtr);
	  break;
	case CDF_INT4:
	  *((Int32 *) toPtr) = (Int32) *((double *) fromPtr);
	  break;
	case CDF_UINT4:
	  *((uInt32 *) toPtr) = (uInt32) *((double *) fromPtr);
	  break;
	case CDF_FLOAT:
	case CDF_REAL4:
	  *((float *) toPtr) = (float) *((double *) fromPtr);
	  break;
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8:
	  *((double *) toPtr) = (double) *((double *) fromPtr);
	  break;
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i, len;
	  char tmp[MAX_nonSTRING_VALUE_LEN+1];
	  sprintf (tmp, "%f", *((double *) fromPtr));
	  for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
	  }
	  break;
	}
      }
      break;
    /**************************************************************************
    * 16-byte floating-point to...
    **************************************************************************/
    case CDF_EPOCH16:
      switch (toDataType) {
        case CDF_BYTE:
        case CDF_INT1:
          *((sChar *) toPtr) = (sChar) *((double *) fromPtr);
	  *(((sChar *) toPtr) + 1) = (sChar) *(((double *) fromPtr) + 1);
          break;
        case CDF_UINT1:
          *((uChar *) toPtr) = (uChar) *((double *) fromPtr);
	  *(((uChar *) toPtr) + 1) = (uChar) *(((double *) fromPtr) + 1);
          break;
        case CDF_INT2:
          *((Int16 *) toPtr) = (Int16) *((double *) fromPtr);
	  *(((Int16 *) toPtr) + 1) = (Int16) *(((double *) fromPtr) + 1);
          break;
        case CDF_UINT2:
          *((uInt16 *) toPtr) = (uInt16) *((double *) fromPtr);
	  *(((uInt16 *) toPtr) + 1) = (uInt16) *(((double *) fromPtr) + 1);
          break;
        case CDF_INT4:
          *((Int32 *) toPtr) = (Int32) *((double *) fromPtr);
	  *(((Int32 *) toPtr) + 1) = (Int32) *(((double *) fromPtr) + 1);
          break;
        case CDF_UINT4:
          *((uInt32 *) toPtr) = (uInt32) *((double *) fromPtr);
	  *(((uInt32 *) toPtr) + 1) = (uInt32) *(((double *) fromPtr) + 1);
          break;
        case CDF_FLOAT:
        case CDF_REAL4:
          *((float *) toPtr) = (float) *((double *) fromPtr);
	  *(((float *) toPtr) + 1) = (float) *(((double *) fromPtr) + 1);
          break;
        case CDF_EPOCH:
        case CDF_DOUBLE:
        case CDF_REAL8:
          *((double *) toPtr) = (double) *((double *) fromPtr);
	  *(((double *) toPtr) + 1) = (double) *(((double *) fromPtr) + 1);
          break;
        case CDF_CHAR:
        case CDF_UCHAR: {
          int i, j, len;
          char tmp[MAX_nonSTRING_VALUE_LEN+1];
          sprintf (tmp, "%f", *((double *) fromPtr));
          for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
             *((char *) toPtr + i) = (i < len ? tmp[i] : ' ');
          }
          sprintf (tmp, "%f", *(((double *) fromPtr) + 1));
	  j = i;
          for (i = 0, len = strlen(tmp); i < toNumElems; i++) {
             *((char *) toPtr + j) = (i < len ? tmp[i] : ' ');
          }
          break;
        }
      }
      break;
    /**************************************************************************
    * character (string) to...
    **************************************************************************/
    case CDF_CHAR:
    case CDF_UCHAR: {
      size_t nBytes = (size_t) (fromNumElems + 1);
      char *str = (char *) cdf_AllocateMemory (nBytes, FatalError);
      memmove (str, fromPtr, (size_t) fromNumElems);
      str[(int)fromNumElems] = NUL;
      switch (toDataType) {
	case CDF_BYTE:
	case CDF_INT1: {
	  int tmp;
	  if (sscanf(str,"%d",&tmp) < 1)
	    *((sChar *) toPtr) = 0;
	  else
	    *((sChar *) toPtr) = (sChar) tmp;
	  break;
	}
	case CDF_UINT1: {
	  uInt tmp;
	  if (sscanf(str,"%u",&tmp) < 1)
	    *((uChar *) toPtr) = 0;
	  else
	    *((uChar *) toPtr) = (uChar) tmp;
	  break;
	}
	case CDF_INT2: {
	  int tmp;
	  if (sscanf(str,"%d",&tmp) < 1)
	    *((Int16 *) toPtr) = 0;
	  else
	    *((Int16 *) toPtr) = (Int16) tmp;
	  break;
	}
	case CDF_UINT2: {
	  uInt tmp;
	  if (sscanf(str,"%u",&tmp) < 1)
	    *((uInt16 *) toPtr) = 0;
	  else
	    *((uInt16 *) toPtr) = (uInt16) tmp;
	  break;
	}
	case CDF_INT4: {
	  long tmp;
	  if (sscanf(str,"%ld",&tmp) < 1)
	    *((Int32 *) toPtr) = 0;
	  else
	    *((Int32 *) toPtr) = (Int32) tmp;
	  break;
	}
	case CDF_UINT4: {
	  uLong tmp;
	  if (sscanf(str,"%lu",&tmp) < 1)
	    *((uInt32 *) toPtr) = 0;
	  else
	    *((uInt32 *) toPtr) = (uInt32) tmp;
	  break;
	}
	case CDF_FLOAT:
	case CDF_REAL4: {
	  float tmp;
	  if (sscanf(str,"%g",&tmp) < 1)
	    *((float *) toPtr) = (float) 0.0;
	  else
	    *((float *) toPtr) = tmp;
	  break;
	}
	case CDF_EPOCH:
	case CDF_DOUBLE:
	case CDF_REAL8: {
	  double tmp;
	  if (sscanf(str,"%lg",&tmp) < 1)
	    *((double *) toPtr) = 0.0;
	  else
	    *((double *) toPtr) = tmp;
	  break;
	}
        case CDF_EPOCH16: {
          double tmp1, tmp2;
          if (sscanf(str,"%lg %lg",&tmp1, &tmp2) < 2) {
            *((double *) toPtr) = 0.0;
	    *(((double *) toPtr) + 1) = 0.0;
          } else {
            *((double *) toPtr) = tmp1;
	    *(((double *) toPtr) + 1) = tmp2;
	  }
          break;
        }
	case CDF_CHAR:
	case CDF_UCHAR: {
	  int i;
	  for (i = 0; i < toNumElems; i++) {
	     *((char *) toPtr + i) = (i < fromNumElems ? str[i] : ' ');
	  }
	  break;
	}
      }
      cdf_FreeMemory (str, FatalError);
      break;
    }
  }
  return;
}

/******************************************************************************
* DecodeValues.
*     Memory for the values is allocated here but is NOT freed (unless an
* error occurred.)
******************************************************************************/

Logical DecodeValues (string, dataType, numElems, values, style)
char *string;
long dataType;
long *numElems;
void **values;
int style;              /* EPOCH style. */
{
if (STRINGdataType(dataType)) {
  /****************************************************************************
  * Delimited character string.
  ****************************************************************************/
  char delim;
  char *firstDelim, *secondDelim;
  char *ptr = string;
  while (*ptr != NUL && Spacing(*ptr)) ptr++;    /* Skip to delimiter. */
  if (*ptr == NUL) return FALSE;                    /* Null-string. */
  delim = *ptr;
  firstDelim = ptr;
  secondDelim = strchr (ptr + 1, delim);
  if (secondDelim == NULL) return FALSE;
  *numElems = secondDelim - firstDelim - 1;
  *values = cdf_AllocateMemory ((size_t) (*numElems + 1), FatalError);
  memmove (*values, firstDelim + 1, (size_t) *numElems);
  ((char *) *values)[(int)(*numElems)] = NUL;
}
else {
  /****************************************************************************
  * Comma separated values.
  ****************************************************************************/
  int i, Ncommas; char *ptr;
  for (Ncommas = 0, i = 0; string[i] != NUL; i++)       /* Count commas. */
     if (string[i] == ',') Ncommas++;
  *numElems = Ncommas + 1;
  *values = cdf_AllocateMemory ((size_t) (*numElems * CDFelemSize(dataType)),
			    FatalError);
  for (ptr = string, i = 0; i < *numElems; i++) {
     switch (dataType) {
       case CDF_BYTE:
       case CDF_INT1: {
	 int temp;
	 if (sscanf(ptr,"%d",&temp) != 1) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 if (temp < -128 || temp > 127) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 *((sChar *) *values + i) = (sChar) temp;
	 break;
       }
       case CDF_UINT1: {
	 int temp;
	 if (sscanf(ptr,"%d",&temp) != 1) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 if (temp < 0 || temp > 255) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 *((uChar *) *values + i) = (uChar) temp;
	 break;
       }
       case CDF_INT2: {
	 if (sscanf(ptr,"%hd",(Int16 *) *values + i) != 1) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 break;
       }
       case CDF_UINT2: {
	 long temp;
	 if (sscanf(ptr,"%ld",&temp) != 1) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 if (temp < 0 || temp > 65535L) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 *((uInt16 *) *values + i) = (uInt16) temp;
	 break;
       }
       case CDF_INT4:
	 if (!DecodeInt32(ptr,(Int32 *) *values + i)) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 break;
       case CDF_UINT4:
	 if (!DecodeInt32u(ptr,(uInt32 *) *values + i)) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 break;
       case CDF_REAL4:
       case CDF_FLOAT:
	 if (sscanf(ptr,"%f",(float *) *values + i) != 1) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 break;
       case CDF_REAL8:
       case CDF_DOUBLE:
	 if (sscanf(ptr,"%lf",(double *) *values + i) != 1) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 break;
       case CDF_EPOCH: {
	 double value;
	 switch (style) {
	   case EPOCH0_STYLE:
	     value = parseEPOCH (ptr);
	     break;
	   case EPOCH1_STYLE:
	     value = parseEPOCH1 (ptr);
	     break;
	   case EPOCH2_STYLE:
	     value = parseEPOCH2 (ptr);
	     break;
	   case EPOCH3_STYLE:
	     value = parseEPOCH3 (ptr);
	     break;
	   case EPOCHf_STYLE:
	     cdf_FreeMemory (*values, FatalError);
	     return DecodeValues(string,CDF_DOUBLE,numElems,values,style);
	   case EPOCHx_STYLE:
	     value = parseEPOCH (ptr);  /* Assume EPOCH0_STYLE instead. */
	     break;
	 }
	 if (value == ILLEGAL_EPOCH_VALUE) {
	   cdf_FreeMemory (*values, FatalError);
	   return FALSE;
	 }
	 else
	   *((double *) *values + i) = value;
	 break;
       }
       case CDF_EPOCH16: {
         double value[2], dummy;
         switch (style) {
           case EPOCH0_STYLE:
             dummy = parseEPOCH16 (ptr, value);
             break;
           case EPOCH1_STYLE:
             dummy = parseEPOCH16_1 (ptr, value);
             break;
           case EPOCH2_STYLE:
             dummy = parseEPOCH16_2 (ptr, value);
             break;
           case EPOCH3_STYLE:
             dummy = parseEPOCH16_3 (ptr, value);
             break;
           case EPOCHf_STYLE:
             cdf_FreeMemory (*values, FatalError);
             return DecodeValues(string,CDF_DOUBLE,numElems,values,style);
           case EPOCHx_STYLE:	/* Assume EPOCH0_STYLE instead. */
             dummy = parseEPOCH16 (ptr, value);
             break;
         }
         if (dummy == ILLEGAL_EPOCH_VALUE ||
	     value[0] == ILLEGAL_EPOCH_VALUE ||
	     value[1] == ILLEGAL_EPOCH_VALUE) {
           cdf_FreeMemory (*values, FatalError);
           return FALSE;
         }
         else {
	   int j = 2 * i;
           *((double *) *values + j) = value[0];
	   *(((double *) *values + j) + 1) = value[1];
	 }
         break;
       }
     }
     ptr = strchr (ptr, ',') + 1;      /* Garbage last time, doesn't matter. */
  }
}
return TRUE;
}

/******************************************************************************
* DecodeInt32.
*    This routine is necessary because DEC Alphas running OpenVMS do not
* decode "-2147483648" properly.
******************************************************************************/

static Logical DecodeInt32 (text, binary)
char *text;
Int32 *binary;
{
  if (EqStringsIgLTws(text,"-2147483648"))
#if defined(dos)
    *binary = (Int32) -2147483648L;
#else
    *binary = (Int32) 0x80000000;
#endif
  else
    if (sscanf(text,Int32FORMAT,binary) != 1) return FALSE;
  return TRUE;
}

/******************************************************************************
* DecodeInt32u.
*    This routine is necessary because VAX/VMS systems do not properly decode
* unsigned values.
******************************************************************************/

static Logical DecodeInt32u (text, binary)
char *text;
uInt32 *binary;
{
   char *start = text;
   char *end, temp[MAX_ENCODED_INT4_LEN+1];
   int len; uLong tempB;
   while (*start != NUL && Spacing(*start)) start++;
   if (*start == NUL) return FALSE;
   end = start;
   while (*end != NUL && Decimal(*end)) end++;
   len = (int) (end - start);
   if (len > MAX_ENCODED_INT4_LEN) return FALSE;
   strcpyX (temp, start, MINIMUM(len,MAX_ENCODED_INT4_LEN));
   if (len < MAX_ENCODED_INT4_LEN) {
     if (sscanf(temp,"%ld",&tempB) != 1) return FALSE;
   }
   else {
     if (strcmp(temp,"4294967295") > 0) return FALSE;
     if (sscanf(&temp[1],"%ld",&tempB) != 1) return FALSE;
#if defined(dos)
     tempB += (1000000000UL * (temp[0] - '0'));
#else
     tempB += ((uLong) 1000000000 * (temp[0] - '0'));
#endif
   }
   *binary = (uInt32) tempB;
   return TRUE;
}

/******************************************************************************
* EqStringsIgLTws.
*     Compares two strings ignoring leading and trailing white space.
******************************************************************************/

static Logical EqStringsIgLTws (string1, string2)
char *string1;
char *string2;
{
  char *tString1, *tString2, *ptr;
  size_t nChars;
  Logical returnValue;
  /****************************************************************************
  * Strip leading and trailing white space from 1st string.
  ****************************************************************************/
  ptr = string1;
  while (*ptr != NUL && Spacing(*ptr)) ptr++;
  nChars = strlen (ptr);
  tString1 = (char *) cdf_AllocateMemory (nChars + 1, FatalError);
  strcpyX (tString1, ptr, 0);
  ptr = tString1 + nChars;
  while (ptr != tString1 && Spacing(*(ptr-1))) ptr--;
  *ptr = NUL;
  /****************************************************************************
  * Strip leading and trailing white space from 2nd string.
  ****************************************************************************/
  ptr = string2;
  while (*ptr != NUL && Spacing(*ptr)) ptr++;
  nChars = strlen (ptr);
  tString2 = (char *) cdf_AllocateMemory (nChars + 1, FatalError);
  strcpyX (tString2, ptr, 0);
  ptr = tString2 + nChars;
  while (ptr != tString2 && Spacing(*(ptr-1))) ptr--;
  *ptr = NUL;
  /****************************************************************************
  * Free temporary strings and return result of comparison.
  ****************************************************************************/
  returnValue = (!strcmp(tString1,tString2) ? TRUE : FALSE);
  cdf_FreeMemory (tString1, FatalError);
  cdf_FreeMemory (tString2, FatalError);
  return returnValue;
}

/******************************************************************************
* LongWidth.
******************************************************************************/

int LongWidth (value)
long value;
{
  if (value < -999999999L) return 11;
  if (value < -99999999L) return 10;
  if (value < -9999999L) return 9;
  if (value < -999999L) return 8;
  if (value < -99999L) return 7;
  if (value < -9999L) return 6;
  if (value < -999L) return 5;
  if (value < -99L) return 4;
  if (value < -9L) return 3;
  if (value < 0L) return 2;
  if (value < 10L) return 1;
  if (value < 100L) return 2;
  if (value < 1000L) return 3;
  if (value < 10000L) return 4;
  if (value < 100000L) return 5;
  if (value < 1000000L) return 6;
  if (value < 10000000L) return 7;
  if (value < 100000000L) return 8;
  if (value < 1000000000L) return 9;
  return 10;
}

/******************************************************************************
* Long64Width.
******************************************************************************/

int Long64Width (value)
OFF_T value;
{
#if !defined(win32) & !defined(__osf__)
  if (value < (OFF_T) -999999999999999999LL) return 20;
  if (value < (OFF_T) -99999999999999999LL) return 19;
  if (value < (OFF_T) -9999999999999999LL) return 18;
  if (value < (OFF_T) -999999999999999LL) return 17;
  if (value < (OFF_T) -99999999999999LL) return 16;
  if (value < (OFF_T) -9999999999999LL) return 15;
  if (value < (OFF_T) -999999999999LL) return 14;
  if (value < (OFF_T) -99999999999LL) return 13;
  if (value < (OFF_T) -9999999999LL) return 12;
  if (value < (OFF_T) -999999999LL) return 11;
  if (value < (OFF_T) -99999999LL) return 10;
  if (value < (OFF_T) -9999999LL) return 9;
  if (value < (OFF_T) -999999LL) return 8;
  if (value < (OFF_T) -99999LL) return 7;
  if (value < (OFF_T) -9999LL) return 6;
  if (value < (OFF_T) -999LL) return 5;
  if (value < (OFF_T) -99LL) return 4;
  if (value < (OFF_T) -9LL) return 3;
  if (value < (OFF_T) 0LL) return 2;
  if (value < (OFF_T) 10LL) return 1;
  if (value < (OFF_T) 100LL) return 2;
  if (value < (OFF_T) 1000LL) return 3;
  if (value < (OFF_T) 10000LL) return 4;
  if (value < (OFF_T) 100000LL) return 5;
  if (value < (OFF_T) 1000000LL) return 6;
  if (value < (OFF_T) 10000000LL) return 7;
  if (value < (OFF_T) 100000000LL) return 8;
  if (value < (OFF_T) 1000000000LL) return 9;
  if (value < (OFF_T) 10000000000LL) return 10;
  if (value < (OFF_T) 100000000000LL) return 11;
  if (value < (OFF_T) 1000000000000LL) return 12;
  if (value < (OFF_T) 10000000000000LL) return 13;
  if (value < (OFF_T) 100000000000000LL) return 14;
  if (value < (OFF_T) 1000000000000000LL) return 15;
  if (value < (OFF_T) 10000000000000000LL) return 16;
  if (value < (OFF_T) 100000000000000000LL) return 17;
  if (value < (OFF_T) 1000000000000000000LL) return 18;
#else
  if (value < (OFF_T) -999999999999999999L) return 20;
  if (value < (OFF_T) -99999999999999999L) return 19;
  if (value < (OFF_T) -9999999999999999L) return 18;
  if (value < (OFF_T) -999999999999999L) return 17;
  if (value < (OFF_T) -99999999999999L) return 16;
  if (value < (OFF_T) -9999999999999L) return 15;
  if (value < (OFF_T) -999999999999L) return 14;
  if (value < (OFF_T) -99999999999L) return 13;
  if (value < (OFF_T) -9999999999L) return 12;
  if (value < (OFF_T) -999999999L) return 11;
  if (value < (OFF_T) -99999999L) return 10;
  if (value < (OFF_T) -9999999L) return 9;
  if (value < (OFF_T) -999999L) return 8;
  if (value < (OFF_T) -99999L) return 7;
  if (value < (OFF_T) -9999L) return 6;
  if (value < (OFF_T) -999L) return 5;
  if (value < (OFF_T) -99L) return 4;
  if (value < (OFF_T) -9L) return 3;
  if (value < (OFF_T) 0L) return 2;
  if (value < (OFF_T) 10L) return 1;
  if (value < (OFF_T) 100L) return 2;
  if (value < (OFF_T) 1000L) return 3;
  if (value < (OFF_T) 10000L) return 4;
  if (value < (OFF_T) 100000L) return 5;
  if (value < (OFF_T) 1000000L) return 6;
  if (value < (OFF_T) 10000000L) return 7;
  if (value < (OFF_T) 100000000L) return 8;
  if (value < (OFF_T) 1000000000L) return 9;
  if (value < (OFF_T) 10000000000L) return 10;
  if (value < (OFF_T) 100000000000L) return 11;
  if (value < (OFF_T) 1000000000000L) return 12;
  if (value < (OFF_T) 10000000000000L) return 13;
  if (value < (OFF_T) 100000000000000L) return 14;
  if (value < (OFF_T) 1000000000000000L) return 15;
  if (value < (OFF_T) 10000000000000000L) return 16;
  if (value < (OFF_T) 100000000000000000L) return 17;
  if (value < (OFF_T) 1000000000000000000L) return 18;
#endif
  return 19;
}

/******************************************************************************
* ParseCacheSizes.
* It can be assumed that there is at least one character in `sizes' or QOP
* would have failed.
******************************************************************************/

Logical  ParseCacheSizes (sizes, workingCache, stageCache, compressCache)
char *sizes;
long *workingCache;
long *stageCache;
long *compressCache;
{
  char *p, *size, tmp[MAX_CACHESIZES_LEN+1];
  /****************************************************************************
  * Initialize all cache sizes in case only some have been specified.
  ****************************************************************************/
  *workingCache = useDEFAULTcacheSIZE;
  *stageCache = useDEFAULTcacheSIZE;
  *compressCache = useDEFAULTcacheSIZE;
  /****************************************************************************
  * Move cache sizes to a temporary buffer and remove all spacing.
  ****************************************************************************/
  strcpyX (tmp, sizes, MAX_CACHESIZES_LEN);
  RemoveWhiteSpace (tmp);
  p = tmp;
  /****************************************************************************
  * Check for (and skip) parenthesis on VMS systems.
  ****************************************************************************/
#if defined(vms)
  if (*p == '(') {
    char *lastChar = p + strlen(p) - 1;
    if (*lastChar != ')') return FALSE;
    p++;
    *lastChar = NUL;
  }
#endif
  /****************************************************************************
  * Scan the list of cache sizes...
  ****************************************************************************/
  for (;;) {
     /*************************************************************************
     * Save the location of the size and increment to the next non-decimal
     * character (ie. past the size).
     *************************************************************************/
     size = p;
     while (Decimal(*p)) p++;
     /*************************************************************************
     * Based on the non-decimal character reached...
     *************************************************************************/
     switch (*p) {
       case 's':
	 if (sscanf(size,"%lds",stageCache) != 1) return FALSE;
	 break;
       case 'c':
	 if (sscanf(size,"%ldc",compressCache) != 1) return FALSE;
	 break;
       case 'd':
	 if (sscanf(size,"%ldd,",workingCache) != 1) return FALSE;
	 break;
       case ',':
	 if (sscanf(size,"%ld,",workingCache) != 1) return FALSE;
	 break;
       case NUL:
	 if (sscanf(size,"%ld",workingCache) != 1) return FALSE;
	 return TRUE;
       default:
	 return FALSE;
     }
     /*************************************************************************
     * Increment to the beginning of the next cache size (or to the end of the
     * sizes).
     *************************************************************************/
     if (*p != ',') p++;
     if (*p == ',') p++;
     if (*p == NUL) return TRUE;
  }
}

/******************************************************************************
*  Remove the ".skt" file extension from the given file name if it's there.
*  It ignores the case.
*  Example:
*      mydata.skt => mydata
******************************************************************************/

void RemoveCDFSktExtension (sktName, sktPath)
char *sktName;         /* CDF skeleton table file name. */
char *sktPath;         /* The string holding file name without extension. */
{
    int ptr = -1;

    strcpyX (sktPath, sktName, CDF_PATHNAME_LEN);
    if (EndsWithIgCase (sktPath, ".skt")) {
      ptr = StrlaststrIgCase(sktPath, ".skt");
      if (ptr != -1) ((char *) sktPath)[ptr] = (char) '\0';
    }
    return;
}


