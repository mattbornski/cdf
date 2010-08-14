/******************************************************************************
*
*  NSSDC/CDF                                                    CDFconvert.
*
*  Version 2.5c, 14-Dec-97, Hughes STX.
*
*  Modification history:
*
*   V1.0  24-Jan-91, H Leckner  Original version (for CDF V2.0).
*   V2.0  17-May-92, J Love     IBM PC port (major changes).
*   V2.1  29-Sep-92, J Love     CDF V2.3 (shareable/NeXT/zVar).
*   V2.1a  6-Oct-92, J Love     Fixed freeing of `bufferX' if 1-dimensional.
*   V2.2  25-Jan-94, J Love     CDF V2.4.
*   V2.3   7-Dec-94, J Love     CDF V2.5.
*   V2.3a 10-Jan-95, J Love     Uppercase file extensions on the Macintosh.
*   V2.3b  6-Apr-95, J Love     POSIX.
*   V2.4   6-Sep-95, J Love     CDFexport-related changes.  Hyper groups.
*   V2.4a 19-Sep-95, J Love     CHECKforABORTso.
*   V2.4b 29-Sep-95, J Love     Macintosh dialog filtering.  Outline default
*                               button.  Less CHECKforABORTso.
*   V2.5   9-Sep-96, J Love     CDF V2.6.
*   V2.5a 21-Feb-97, J Love	Removed RICE.
*   V2.5b 17-Nov-97, J Love	Windows NT/Visual C++.
*   V2.5c 14-Dec-97, J Love	Added ALPHAVMSi encoding.
*   V2.6  17-Apr-01, M Liu      Added checking for variable names entered in
*                               the compression option
*   V2.7  03-May-06, M Liu      Added checksum option for the converted files.
*   V2.8  13-Oct-06, M Liu      Changed to allow upper and lower case CDF  
*                               name to be used on win32.
*   V3.3  10-Apr-09, M Liu      Changed MAC_ENCODING to PPC_ENCODING.
*
******************************************************************************/
#include "cdfcvt.h"
#include "cdflib.h"
#include "cdflib64.h"

/******************************************************************************
* Increased stack size for Borland C on IBM PC.
******************************************************************************/

#if defined(BORLANDC)
extern unsigned _stklen = BORLANDC_STACK_SIZE;
#endif

/******************************************************************************
* Global variables.
******************************************************************************/

Logical useSkeletonCDF;
Logical mLog, pctLog;
Logical deleteIFexists;
Logical negToPosFp0;
long dstFormat;
long dstMajority;
long dstEncoding;
long srcFormat, srcMajority, srcEncoding, srcVersion, srcRelease, srcIncrement;
CDFid srcId, dstId, sktId;
long zMode;
Logical report[3];
Logical backward = FALSE;
int checksumFlag;
long checksum, srcChecksum = -999;
long workingCache, stageCache, compressCache;
Logical dumpStatistics;
struct CompressionStruct *compression;
struct SparseRecordsStruct *sparseRecords;
Logical pctOn;

/******************************************************************************
* Main.
******************************************************************************/

#if !defined(win32) || (defined(win32) && defined(ALONE))
MAIN {
  Logical success = TRUE;
  strcpyX (pgmName, "CDFconvert", MAX_PROGRAM_NAME_LEN);
#if defined(mac)
  MacExecuteSO (ConvertCDFs, ConvertQOPs);
#else
  success = ConvertCDFs (argc, argv);
#endif
#if defined(DEBUG)
  if (cdf_FreeMemory(NULL,FatalError) > 0) DisplayWarning ("Abandoned buffers.");
#else
  cdf_FreeMemory (NULL, FatalError);
#endif
  return BOO(success,EXIT_SUCCESS_,EXIT_FAILURE_);
}
#endif

/******************************************************************************
* ConvertCDFs.
******************************************************************************/

Logical ConvertCDFs (argC, argV)
int argC;
char *argV[];
{
  QOP *qop;
  static char *validQuals[] = {
    "single", "multi", "row", "column", "host", "network", "skeleton", "log",
    "nolog", "percent", "nopercent", "delete", "nodelete", "zmode",
    "neg2posfp0", "noneg2posfp0", "report", "page", "nopage", "cache",
    "statistics", "nostatistics", "encoding", "compression", "sparseness",
    "about", "backward", "checksum", NULL };
  static int optRequired[] = {
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE,
    TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, 0 };
  static char *reportTokens[] = { "errors", "warnings", "informationals" };
  CDFstatus status;
  char srcSpec[DU_MAX_PATH_LEN+1];
  char dstSpec[DU_MAX_PATH_LEN+1];
  char srcPath[DU_MAX_PATH_LEN+1];
  char dstPath[DU_MAX_PATH_LEN+1];
  char sktPath[DU_MAX_PATH_LEN+1];
  char CDFname[DU_MAX_NAME_LEN+1];
  long numCDFs;
  int i;
  char **directories = NULL;
  char **CDFnames = NULL;
  Logical qopError = FALSE;
  /****************************************************************************
  * Set global variables.
  ****************************************************************************/
  dstFormat = SOURCEformat;
  dstMajority = SOURCEmajority;
  dstEncoding = SOURCEencoding;
  /****************************************************************************
  * Get qualifiers/options/parameters.
  ****************************************************************************/
  switch (argC) {
    case 1:
      PageOLH ("cdfcvt.olh", argV[0]);
      return TRUE;
    case 2:
      if (strcmp(argV[1],"-java") == 0) {
        pagingOn = FALSE;
        PageOLH ("cdfcvtj.olh", argV[0]);
        return TRUE;
      }
    default:
      qop = Qop (argC, argV, validQuals, optRequired);
      if (qop == NULL) return FALSE;
      /************************************************************************
      * Check for `about' qualifier.
      ************************************************************************/
      if (qop->qualEntered[ABOUTqual]) {
	DisplayIdentification (pgmName);
	cdf_FreeMemory (qop, FatalError);
	return TRUE;
      }
      /************************************************************************
      * Get CDFs/paths to convert.
      ************************************************************************/
      switch (qop->Nparms) {
	case 0:
	  DisplayError ("Missing source and destination CDF specifications.");
	  qopError = TRUE;
	  break;
	case 1:
	  DisplayError ("Missing source or destination CDF specification.");
	  qopError = TRUE;
	  break;
	case 2:
	  strcpyX (srcSpec, qop->parms[CDFSPEC1parm], DU_MAX_PATH_LEN);
	  strcpyX (dstSpec, qop->parms[CDFSPEC2parm], DU_MAX_PATH_LEN);
#if defined(vms) || defined(dos)
	  MakeUpperString (srcSpec);
	  MakeUpperString (dstSpec);
#endif
	  break;
	default:
	  DisplayError ("Too many parameters.");
	  qopError = TRUE;
	  break;
      }
      if (IsWild(dstSpec)) {
	DisplayError ("Destination cannot be a wildcard specification.");
	qopError = TRUE;
      }
      if (IsDir(srcSpec) || IsWild(srcSpec)) {
	if (!IsDir(dstSpec)) {
	  DisplayError ("Destination must be a directory.");
	  qopError = TRUE;
	}
      }
      /************************************************************************
      * Check for skeleton CDF qualifier.
      ************************************************************************/
      if (qop->qualEntered[SKELqual])
	strcpyX (sktPath, qop->qualOpt[SKELqual], DU_MAX_PATH_LEN);
      else
	strcpyX (sktPath, "", DU_MAX_PATH_LEN);
      /************************************************************************
      * Check for an overriding format, majority, or encoding qualifier.
      ************************************************************************/
      qopError = qopError | !S2qualifierLong(qop,&dstFormat,SINGLEqual,
					     SINGLE_FILE,MULTIqual,MULTI_FILE,
					     dstFormat,"format");
      qopError = qopError | !S2qualifierLong(qop,&dstMajority,ROWqual,
					     ROW_MAJOR,COLqual,COLUMN_MAJOR,
					     dstMajority,"majority");
      qopError = qopError | !S2qualifierLong(qop,&dstEncoding,HOSTqual,
					     HOST_ENCODING,NETqual,
					     NETWORK_ENCODING,dstEncoding,
					     "encoding");
      if (qop->qualEntered[ENCODINGqual]) {
	if (dstEncoding != SOURCEencoding) {
	  DisplayError ("Conflicting qualifiers (encoding/host/network).");
	  qopError = TRUE;
	}
	else {
	  static char *encodingStrings[] = {
	    "host", "network", "sun", "vax", "decstation", "sgi",
	    "ibmpc", "ibmrs", "mac", "hp", "next", "alphaosf1",
	    "alphavmsd", "alphavmsg", "alphavmsi", NULL
	  };
	  static long encodings[] = {
	    HOST_ENCODING, NETWORK_ENCODING, SUN_ENCODING, VAX_ENCODING,
	    DECSTATION_ENCODING, SGi_ENCODING, IBMPC_ENCODING, IBMRS_ENCODING,
	    PPC_ENCODING, HP_ENCODING, NeXT_ENCODING, ALPHAOSF1_ENCODING,
	    ALPHAVMSd_ENCODING, ALPHAVMSg_ENCODING, ALPHAVMSi_ENCODING
	  };
	  int match = FindUniqueMatch (qop->qualOpt[ENCODINGqual],
				       encodingStrings);
	  switch (match) {
	    case NOMATCH:
	      DisplayError ("Unknown encoding.");
	      qopError = TRUE;
	      break;
	    case MATCHES:
	      DisplayError ("Ambiguous encoding.");
	      qopError = TRUE;
	      break;
	    default:
	      dstEncoding = encodings[match];
	      break;
	  }
	}
      }
      /************************************************************************
      * Check for /ZMODE,-zmode qualifier.
      ************************************************************************/
      if (qop->qualEntered[ZMODEqual]) {
	switch (qop->qualOpt[ZMODEqual][0]) {
	  case '0': zMode = zMODEoff; break;
	  case '1': zMode = zMODEon1; break;
	  case '2': zMode = zMODEon2; break;
	  default: {
	    DisplayError ("Illegal zMode.");
	    qopError = TRUE;
	  }
	}
      }
      else
	zMode = DEFAULTzModeCVT;
      /*********************************************************************
      * Check for `cache' qualifier.
      *********************************************************************/
      if (qop->qualEntered[CACHEqual]) {
	if (!ParseCacheSizes(qop->qualOpt[CACHEqual],
			     &workingCache,&stageCache,&compressCache)) {
	  DisplayError ("Illegal cache size/type.");
	  qopError = TRUE;
	}
      }
      else {
	workingCache = useDEFAULTcacheSIZE;
	stageCache = useDEFAULTcacheSIZE;
	compressCache = useDEFAULTcacheSIZE;
      }
      /************************************************************************
      * Check for `report' qualifier.  If absent, use defaults.
      ************************************************************************/
      if (qop->qualEntered[REPORTqual]) {
	if (!ParseOptionList(3,reportTokens,qop->qualOpt[REPORTqual],report)) {
	  DisplayError ("Illegal list of `report' options.");
	  qopError = TRUE;
	}
      }
      else {
	report[ERRORs] = REPORTerrorsDEFAULT;
	report[WARNs] = REPORTwarningsDEFAULT;
	report[INFOs] = REPORTinfosDEFAULT;
      }
      /************************************************************************
      * Check for `compression' qualifier.
      ************************************************************************/
      if (qop->qualEntered[COMPRESSqual]) {
	compression = ParseCompressionOptions (qop->qualOpt[COMPRESSqual]);
	if (compression == NULL) {
	  DisplayError ("Illegal `compression' options.");
	  qopError = TRUE;
	}
      }
      else
	compression = NULL;
      /************************************************************************
      * Check for `sparseness' qualifier.
      *
      * NOTE: When sparse arrays are added the call to `ParseSparsenessOptions'
      * will have to be changed to return both `sparseRecords' and
      * `sparseArrays' (or something like that).
      ************************************************************************/
      if (qop->qualEntered[SPARSEqual]) {
	sparseRecords = ParseSparsenessOptions (qop->qualOpt[SPARSEqual]);
	if (sparseRecords == NULL) {
	  DisplayError ("Illegal `sparseness' options.");
	  qopError = TRUE;
	}
      }
      else
	sparseRecords = NULL;
      /************************************************************************
      * Check for `backward' qualifier.
      ************************************************************************/
      if (qop->qualEntered[BACKWARDqual]) {
        CDFsetFileBackward(BACKWARDFILEon);
        backward = TRUE;
      }
      /************************************************************************
      * Check for `checksum' qualifier.
      ************************************************************************/
      if (qop->qualEntered[CHECKSUMqual]) {
        char *cks = qop->qualOpt[CHECKSUMqual];
	if (StrStrIgCaseX(cks, "no") || StrStrIgCaseX(cks, "none")) {
          checksumFlag = 0;
	  checksum = NO_CHECKSUM;
	} else if (StrStrIgCaseX(cks, "md5")) {
           checksumFlag = 1;
           checksum = MD5_CHECKSUM;
        } else if (StrStrIgCaseX(cks, "source")) {
           checksumFlag = 2;
        } else {
            DisplayError ("Illegal checksum method.");
            qopError = TRUE;
        }
      }
      else
        checksumFlag = -1;

      /************************************************************************
      * Check for `page', `log', `percent', `delete', `neg2posfp0', and
      * `statistics' qualifiers.
      ************************************************************************/
      qopError = qopError | !TFqualifier (qop,&pagingOn,PAGEqual,NOPAGEqual,
					  DEFAULTpageCVT,"page");
      qopError = qopError | !TFqualifier (qop,&mLog,LOGqual,NOLOGqual,
					  DEFAULTlogCVT,"log");
      qopError = qopError | !TFqualifier (qop,&pctLog,PCTqual,NOPCTqual,
					  DEFAULTpctCVT,"percent");
      qopError = qopError | !TFqualifier (qop,&deleteIFexists,DELqual,
					  NODELqual,DEFAULTdelCVT,"delete");
      qopError = qopError | !TFqualifier (qop,&negToPosFp0,NEG2POSqual,
					  NONEG2POSqual,DEFAULT_NEGtoPOSfp0,
					  "neg2posfp0");
      qopError = qopError | !TFqualifier (qop,&dumpStatistics,STATSqual,
					  NOSTATSqual,DEFAULTstatsCVT,
					  "neg2posfp0");
      /************************************************************************
      * Check for qualifier compatibility.
      ************************************************************************/
      if (pctLog && (!mLog)) {
	DisplayError ("`log' must be used with `percentage'.");
	qopError = TRUE;
      }
      /************************************************************************
      * Free QOP memory and check for an error.
      ************************************************************************/
      cdf_FreeMemory (qop, FatalError);
      if (qopError) {
	if (compression != NULL) FreeCompression (compression);
	if (sparseRecords != NULL) FreeSparseness (sparseRecords);
	return FALSE;
      }
      break;
  }

  /****************************************************************************
  * If a skeleton CDF was specified, open it and set the destination format,
  * majority, and encoding (unless they were explicitly specified).
  ****************************************************************************/
  if (!NULstring(sktPath)) {
    long sktFormat, sktMajority, sktEncoding;
#if defined(vms) || defined(dos)
    MakeUpperString (sktPath);
#endif
    WriteOut (stdout, "Opening Skeleton CDF: ");
    WriteOut (stdout, sktPath);
    WriteOut (stdout, "\n");
    status = CDFlib (OPEN_, CDF_, sktPath, &sktId,
		     GET_, CDF_FORMAT_, &sktFormat,
			   CDF_MAJORITY_, &sktMajority,
			   CDF_ENCODING_, &sktEncoding,
			   CDF_VERSION_, &srcVersion,
                           CDF_RELEASE_, &srcRelease,
                           CDF_INCREMENT_, &srcIncrement,
		     NULL_);
    if (StatusBAD(status)) {
       StatusHandlerCvt ("SKT", status);
       useSkeletonCDF = FALSE;
    }
    if (!PriorTo ("3.2.0", srcVersion, srcRelease, srcIncrement)) {
      status = CDFlib (CDF_CHECKSUM_, &srcChecksum,
                       NULL_);
      if (StatusBAD(status)) {
         StatusHandlerCvt ("SKT", status);
         useSkeletonCDF = FALSE;
      }
    }
    else {
      useSkeletonCDF = TRUE;
      if (dstFormat == SOURCEformat) dstFormat = sktFormat;
      if (dstMajority == SOURCEmajority) dstMajority = sktMajority;
      if (dstEncoding == SOURCEencoding) dstEncoding = sktEncoding;
    }
  }
  else
    useSkeletonCDF = FALSE;
  /****************************************************************************
  * Convert CDFs.
  ****************************************************************************/
  if (IsDir(srcSpec) || IsWild(srcSpec)) {
    numCDFs = CDFdirList (srcSpec, &directories, &CDFnames);
    if (numCDFs < 1) {
      WriteOut (stdout, "No CDFs found in source directory.\n");
    }
    else {
      for (i = 0; i < numCDFs; i++) {
	 strcpyX (srcPath, directories[i], DU_MAX_PATH_LEN);
	 AppendToDir (srcPath, CDFnames[i]);
	 strcpyX (dstPath, dstSpec, DU_MAX_PATH_LEN);
	 AppendToDir (dstPath, CDFnames[i]);
	 ConvertCDF (srcPath, dstPath);
      }
    }
  }
  else {
    if (IsDir(dstSpec)) {
      ParsePath (srcSpec, NULL, CDFname);
      strcpyX (dstPath, dstSpec, DU_MAX_PATH_LEN);
      AppendToDir (dstPath, CDFname);
      ConvertCDF (srcSpec, dstPath);
    }
    else {
      ConvertCDF (srcSpec, dstSpec);
    }
  }
  if (directories != NULL) cdf_FreeMemory (directories, FatalError);
  if (CDFnames != NULL) cdf_FreeMemory (CDFnames, FatalError);
  if (compression != NULL) FreeCompression (compression);
  if (sparseRecords != NULL) FreeSparseness (sparseRecords);
  /****************************************************************************
  * Close skeleton CDF if one was used.
  ****************************************************************************/
  if (useSkeletonCDF) {
    status = CDFclose (sktId);
    StatusHandlerCvt ("SKT", status);	/* Ignore return. */
  }
  return TRUE;
}

/******************************************************************************
* ConvertCDF.
******************************************************************************/

Logical ConvertCDF (srcPath, dstPath)
char *srcPath;
char *dstPath;
{
  return ConvertCDFx (srcPath, dstPath);
}

/******************************************************************************
* ConvertCDFx.
******************************************************************************/

Logical ConvertCDFx (srcPath, dstPath)
char *srcPath;
char *dstPath;
{
  CDFstatus status; int pN;
  long format, majority, encoding, version, release, increment;
  long numDims, dimSizes[CDF_MAX_DIMS];
  long SRCcType, SRCcParms[CDF_MAX_PARMS], SRCcPct;
  long cType, cParms[CDF_MAX_PARMS];
  long numVars, numZvars;
  char text[MAX_OUTPUT_TEXT_LEN+1];
  /***************************************************************************
  * Display converting message.
  ***************************************************************************/
  WriteOut (stdout, "Converting \"");
  WriteOut (stdout, srcPath);
  if (!EndsWithIgCase(srcPath, ".cdf"))
    WriteOut (stdout, ".cdf");
  WriteOut (stdout, "\" to \"");
  WriteOut (stdout, dstPath);
  if (!EndsWithIgCase(dstPath, ".cdf"))
    WriteOut (stdout, ".cdf");
  WriteOut (stdout, "\"\n");
  
  /***************************************************************************
  * Open source CDF.  If an error occurs, skip to next CDF.
  ***************************************************************************/
  if (mLog) WriteOut (stdout, "  Opening source CDF...\n");
  status = CDFlib (OPEN_, CDF_, srcPath, &srcId,
                   GET_, CDF_NUMrVARS_, &numVars,                     
		         CDF_NUMzVARS_, &numZvars,
	           NULL_);
  if (!StatusHandlerCvt("SRC",status)) return FALSE;
  status = CDFlib (SELECT_, CDF_, srcId,
		   SELECT_, CDF_READONLY_MODE_, READONLYon,
			    CDF_zMODE_, zMode,
			    CDF_NEGtoPOSfp0_MODE_, BOO(negToPosFp0,
						       NEGtoPOSfp0on,
						       NEGtoPOSfp0off),
			    CDF_CACHESIZE_, workingCache,
			    STAGE_CACHESIZE_, stageCache,
			    COMPRESS_CACHESIZE_, compressCache,
		   GET_, CDF_FORMAT_, &srcFormat,
                         CDF_VERSION_, &version,
                         CDF_RELEASE_, &release,
                         CDF_INCREMENT_, &increment,
		         CDF_NUMrVARS_, &numVars,
		         CDF_NUMzVARS_, &numZvars,
			 CDF_MAJORITY_, &srcMajority,
			 CDF_ENCODING_, &srcEncoding,
			 CDF_COMPRESSION_, &SRCcType, SRCcParms, &SRCcPct,
			 rVARs_NUMDIMS_, &numDims,
			 rVARs_DIMSIZES_, dimSizes,
		   NULL_);
  if (!StatusHandlerCvt("SRC",status)) return FALSE;
  if (!PriorTo ("3.2.0", version, release, increment)) {
    status = CDFlib (GET_, CDF_CHECKSUM_, &srcChecksum,
                     NULL_);
    if (!StatusHandlerCvt("SRC",status)) return FALSE;
  }
  CHECKforABORTso
  if (CDFgetFileBackward()) {
    /*************************************************************************
    * Check the size of source CDF.  Give errors if it's over 2G when
    * converting it to V2.7 version. It's only applicable to V3.* files.
    *************************************************************************/
    if (version >= 3) {
      OFF_T cSize, uSize;
      status = CDFlib (GET_, CDF_INFO_, srcPath, &SRCcType, SRCcParms,
                                        &cSize, &uSize,
                       NULL_);
      if (!StatusHandlerCvt("SRC",status)) return FALSE;
#if defined(win32)
      if ((cSize >= (OFF_T) ((1i64 << 31) - 1)) ||
          (uSize >= (OFF_T) ((1i64 << 31) - 1))) {
#else
      if ((cSize >= (OFF_T) ((1LL << 31) - 1)) ||
          (uSize >= (OFF_T) ((1LL << 31) - 1))) {
#endif
        WriteOut (stdout, "Soruce file size: ");
#if !defined(win32)
        sprintf (text, "cSize(%lld) uSize(%lld)", cSize, uSize);
#else
        sprintf (text, "cSize(%I64d) uSize(%I64d)", cSize, uSize);
#endif
        WriteOut (stdout, text);
        WriteOut (stdout, " -- is too big to be converted to a V2.7 file. \n");
	status = CDFlib (CLOSE_, CDF_,
		         NULL_);
        return FALSE;
      }
    }
  }

  /***************************************************************************
  * Determine format, majority, encoding, and compression of destination CDF.
  ***************************************************************************/
  format = BOO(dstFormat == SOURCEformat,srcFormat,dstFormat);
  majority = BOO(dstMajority == SOURCEmajority,srcMajority,dstMajority);
  encoding = BOO(dstEncoding == SOURCEencoding,srcEncoding,dstEncoding);
  cType = SRCcType;
  for (pN = 0; pN < CDF_MAX_PARMS; pN++) cParms[pN] = SRCcParms[pN];
  if (compression != NULL) {
    if (compression->CDF.cType != SOURCEcompression) {
      cType = compression->CDF.cType;
      for (pN = 0; pN < CDF_MAX_PARMS; pN++) {
	 cParms[pN] = compression->CDF.cParms[pN];
      }
    }
  }
  /***************************************************************************
  * Create destination CDF.  Note that the cache size is selected after the
  * CDF's format is specified.  This is because changing a CDF's format will
  * also cause it's cache size to be changed.
  ***************************************************************************/
  if (mLog) WriteOut (stdout, "  Creating destination CDF...\n");
  status = CDFlib (CREATE_, CDF_, dstPath, numDims, dimSizes, &dstId,
		   NULL_);
  if (status == CDF_EXISTS && deleteIFexists) {
    status = CDFlib (OPEN_, CDF_, dstPath, &dstId,
		     DELETE_, CDF_,
		     NULL_);
    if (!StatusHandlerCvt("DST",status)) {
      WriteOut (stdout, "  Unabled to delete existing destination CDF.\n");
      if (mLog) WriteOut (stdout, "  Closing source CDF...\n");
      StatusHandlerCvt ("SRC", CDFclose(srcId));
      return FALSE;
    }
    status = CDFlib (CREATE_, CDF_, dstPath, numDims, dimSizes, &dstId,
		     NULL_);
  }
  if (!StatusHandlerCvt("DST",status)) {
    if (mLog) WriteOut (stdout, "  Closing source CDF...\n");
    StatusHandlerCvt ("SRC", CDFclose(srcId));
    return FALSE;
  }
  CHECKforABORTso
  status = CDFlib (SELECT_, CDF_, dstId,
		   PUT_, CDF_FORMAT_, format,
			 CDF_MAJORITY_, majority,
			 CDF_ENCODING_, encoding,
			 CDF_COMPRESSION_, cType, cParms,
		   SELECT_, CDF_CACHESIZE_, workingCache,
			    STAGE_CACHESIZE_, stageCache,
			    COMPRESS_CACHESIZE_, compressCache,
		   NULL_);
  if (!StatusHandlerCvt("DST",status)) {
    if (mLog) WriteOut (stdout, "  Closing destination CDF...\n");
    StatusHandlerCvt ("DST", CDFclose(dstId));
    if (mLog) WriteOut (stdout, "  Closing source CDF...\n");
    StatusHandlerCvt ("SRC", CDFclose(srcId));
    return FALSE;
  }
  CHECKforABORTso

  if (!backward) {
    if (checksumFlag == 0 || checksumFlag == 1) 	/* no or md5 */
      status = CDFlib (SELECT_, CDF_, dstId,
                       PUT_, CDF_CHECKSUM_, checksum,
                       NULL_);
    else if (checksumFlag == 2) 			/* source */
      if (srcChecksum != -999) {
        status = CDFlib (SELECT_, CDF_, dstId,
                         PUT_, CDF_CHECKSUM_, srcChecksum,
                         NULL_);
      }
    else {					/* not specified */
      int env = CDFgetChecksumEnvVar();
      if (env == 0) 				/* env var not set */
        status = CDFlib (SELECT_, CDF_, dstId,
                         PUT_, CDF_CHECKSUM_, srcChecksum,
                         NULL_);
      else					/* env var set to md5 */
        status = CDFlib (SELECT_, CDF_, dstId,
                         PUT_, CDF_CHECKSUM_, MD5_CHECKSUM,
                         NULL_);
    }
  }
  if (!StatusHandlerCvt("DST",status)) {
    if (mLog) WriteOut (stdout, "  Closing destination CDF...\n");
    StatusHandlerCvt ("DST", CDFclose(dstId));
    if (mLog) WriteOut (stdout, "  Closing source CDF...\n");
    StatusHandlerCvt ("SRC", CDFclose(srcId));
    return FALSE;
  }
  CHECKforABORTso
  /***************************************************************************
  * Convert attributes.  If that is successful, convert variables.
  ***************************************************************************/
  if (!ConvertAttributes()) {
    if (mLog) WriteOut (stdout, "  Closing source CDF...\n");
    StatusHandlerCvt ("SRC", CDFclose(srcId));
    if (mLog) WriteOut (stdout, "  Closing destination CDF...\n");
    StatusHandlerCvt ("DST", CDFclose(dstId));
    return FALSE;
  }
  /***************************************************************************
  * Convert variables.
  ***************************************************************************/
  if (!ConvertVariables (srcMajority, majority)) {
    if (mLog) WriteOut (stdout, "  Closing source CDF...\n");
    StatusHandlerCvt ("SRC", CDFclose(srcId));
    if (mLog) WriteOut (stdout, "  Closing destination CDF...\n");
    StatusHandlerCvt ("DST", CDFclose(dstId));
    return FALSE;
  }
  /***************************************************************************
  * Close source and destination CDFs.
  ***************************************************************************/
  if (dumpStatistics) {
    vSTATS vStatsDotCDF, vStatsStage, vStatsCompress;
    if (mLog) WriteOut (stdout, "  Closing source CDF...\n");
    status = CDFlib (SELECT_, CDF_, srcId,
		     CLOSE_, CDFwithSTATS_, &vStatsDotCDF,
					    &vStatsStage,
					    &vStatsCompress,
		     NULL_);
    StatusHandlerCvt ("SRC", status);
    CHECKforABORTso
    DisplayStatistics ("source", &vStatsDotCDF, &vStatsStage, &vStatsCompress);
    if (mLog) WriteOut (stdout, "  Closing destination CDF...\n");
    status = CDFlib (SELECT_, CDF_, dstId,
		     CLOSE_, CDFwithSTATS_, &vStatsDotCDF,
					    &vStatsStage,
					    &vStatsCompress,
		     NULL_);
    StatusHandlerCvt ("DST", status);
    DisplayStatistics ("destination", &vStatsDotCDF, &vStatsStage,
		       &vStatsCompress);
  }
  else {
    if (mLog) WriteOut (stdout, "  Closing source CDF...\n");
    status = CDFlib (SELECT_, CDF_, srcId,
		     CLOSE_, CDF_,
		     NULL_);
    StatusHandlerCvt ("SRC", status);
    CHECKforABORTso
    if (mLog) WriteOut (stdout, "  Closing destination CDF...\n");
    status = CDFlib (SELECT_, CDF_, dstId,
		     CLOSE_, CDF_,
		     NULL_);
    StatusHandlerCvt ("DST", status);
  }
  return TRUE;
}

/******************************************************************************
* ConvertAttributes.
******************************************************************************/

Logical ConvertAttributes ()
{
  CDFstatus status;
  long attrNum, entryNum, ignoredNum;
  long numAttrs, maxEntry, maxZentry;
  long scope;
  char attrName[CDF_ATTR_NAME_LEN256 + 1];

  /****************************************************************************
  * Inquire number of attributes to be converted.
  ****************************************************************************/

  if (mLog) WriteOut (stdout, "  Converting attributes...\n");

  status = CDFlib (SELECT_, CDF_, srcId,
		   GET_, CDF_NUMATTRS_, &numAttrs,
		   NULL_);
  if (!StatusHandlerCvt("SRC",status)) return FALSE;

  for (attrNum = 0; attrNum < numAttrs; attrNum++) {
     /*************************************************************************
     * Inquire attribute in source CDF.
     *************************************************************************/
  
     status = CDFlib (SELECT_, CDF_, srcId,
			       ATTR_, attrNum,
		      GET_, ATTR_NAME_, attrName,
			    ATTR_SCOPE_, &scope,
		      NULL_);
     if (!StatusHandlerCvt("SRC",status)) return FALSE;

     if (mLog) {
       WriteOut (stdout, "    Converting ");
       WriteOut (stdout, BOO(scope == GLOBAL_SCOPE,"g","v"));
       WriteOut (stdout, "Attribute \"");
       WriteOut (stdout, attrName);
       WriteOut (stdout, "\"\n");
     }

     /*************************************************************************
     * Create new attribute in destination CDF.
     *************************************************************************/

     status = CDFlib (SELECT_, CDF_, dstId,
		      CREATE_, ATTR_, attrName, scope, &ignoredNum,
		      NULL_);
     if (!StatusHandlerCvt("DST",status)) return FALSE;

     /*************************************************************************
     * Copy entries from source CDF to destination CDF.
     *************************************************************************/

     if (scope == GLOBAL_SCOPE) {
       status = CDFlib (SELECT_, CDF_, srcId,
			GET_, ATTR_MAXgENTRY_, &maxEntry,
			NULL_);
       if (!StatusHandlerCvt("SRC",status)) return FALSE;
       if (mLog) {
	 WriteOut (stdout, "      Converting entries...\n");
       }
       for (entryNum = 0; entryNum <= maxEntry; entryNum++) {
	  if (!ConvertEntry(entryNum,gENTRYt)) return FALSE;
       }
     }
     else {
       status = CDFlib (SELECT_, CDF_, srcId,
			GET_, ATTR_MAXrENTRY_, &maxEntry,
			      ATTR_MAXzENTRY_, &maxZentry,
			NULL_);
       if (!StatusHandlerCvt("SRC",status)) return FALSE;
       if (mLog) {
	 WriteOut (stdout, "      Converting entries...\n");
       }
       for (entryNum = 0; entryNum <= maxEntry; entryNum++) {
	  if (!ConvertEntry(entryNum,rENTRYt)) return FALSE;
       }
       for (entryNum = 0; entryNum <= maxZentry; entryNum++) {
	  if (!ConvertEntry(entryNum,zENTRYt)) return FALSE;
       }
     }
     CHECKforABORTso
  }

  return TRUE;
}

/******************************************************************************
* ConvertEntry.
******************************************************************************/

Logical ConvertEntry (entryNum, entryType)
long entryNum;
int entryType;
{
  CDFstatus status;
  long dataType;
  long numElements;
  void *value;

  status = CDFlib (SELECT_, CDF_, srcId,
			    ENTRY(entryType), entryNum,
		   GET_, ENTRY_DATATYPE(entryType), &dataType,
			 ENTRY_NUMELEMS(entryType), &numElements,
		   NULL_);
  if (status != NO_SUCH_ENTRY) {
    if (!StatusHandlerCvt("SRC",status)) return FALSE;
    value = cdf_AllocateMemory ((size_t) (CDFelemSize(dataType) * numElements),
			    FatalError);
    status = CDFlib (SELECT_, CDF_, srcId,
		     GET_, ENTRY_DATA(entryType), value,
		     NULL_);
    if (!StatusHandlerCvt("SRC",status)) return FALSE;
    status = CDFlib (SELECT_, CDF_, dstId,
			      ENTRY(entryType), entryNum,
		     PUT_, ENTRY_DATA(entryType), dataType, numElements, value,
		     NULL_);
    if (!StatusHandlerCvt("DST",status)) return FALSE;
    cdf_FreeMemory (value, FatalError);
  }
  CHECKforABORTso
  return TRUE;
}

/******************************************************************************
* ConvertVariables.
******************************************************************************/

Logical ConvertVariables (SRCmajority, DSTmajority)
long SRCmajority;
long DSTmajority;
{
  CDFstatus status;
  long numVars, numZvars;
  long varNum;

  if (mLog) WriteOut (stdout, "  Converting variables...\n");

  /****************************************************************************
  * Inquire number of variables to be converted.
  ****************************************************************************/

  status = CDFlib (SELECT_, CDF_, srcId,
		   GET_, CDF_NUMrVARS_, &numVars,
			 CDF_NUMzVARS_, &numZvars,
		   NULL_);
  if (!StatusHandlerCvt("SRC",status)) return FALSE;

  if (compression != NULL) CheckEnteredVarNames();
  for (varNum = 0; varNum < numVars; varNum++) {
     if (!ConvertVariable(varNum,FALSE,SRCmajority,DSTmajority)) return FALSE;
  }

  for (varNum = 0; varNum < numZvars; varNum++) {
     if (!ConvertVariable(varNum,TRUE,SRCmajority,DSTmajority)) return FALSE;
  }

  return TRUE;
}

/******************************************************************************
* CheckEnteredVarNames.
******************************************************************************/

void CheckEnteredVarNames ()
{
  CDFstatus status; 
  struct cVarStruct *Var;
  long numVars, numZvars;
  long varNum;
  Logical found;
  char varName[CDF_VAR_NAME_LEN256 + 1];

  /****************************************************************************
  * Inquire number of variables to be checked.
  ****************************************************************************/

  status = CDFlib (SELECT_, CDF_, srcId,
                   GET_, CDF_NUMrVARS_, &numVars,
                         CDF_NUMzVARS_, &numZvars,
                   NULL_);
  if (!StatusHandlerCvt("SRC",status)) return;

  for (Var = compression->VARhead; Var != NULL; Var = Var->next) {
     found = FALSE;
     for (varNum = 0; varNum < numVars; varNum++) {
       status = CDFlib (SELECT_, CDF_, srcId,
                                 rVAR_, varNum,
                        GET_, rVAR_NAME_, varName,
                        NULL_);
       if (!StatusHandlerCvt("SRC",status)) return;
       if (!strcmpITB(Var->name,varName)) {
         found = TRUE;
         break;
       }
     }
     if (found) continue;
     for (varNum = 0; varNum < numZvars; varNum++) {
       status = CDFlib (SELECT_, CDF_, srcId,
                                 zVAR_, varNum,
                        GET_, zVAR_NAME_, varName,
                        NULL_);
       if (!StatusHandlerCvt("SRC",status)) return;
       if (!strcmpITB(Var->name,varName)) {
         found = TRUE;
         break;
       }
     }
     if (found) continue;
     WriteOut (stdout, "Warning: variable name: ");
     WriteOut (stdout, Var->name);
     WriteOut (stdout, " not found in the CDF...\n");
  }
}

/******************************************************************************
* ConvertVariable.
******************************************************************************/

Logical ConvertVariable (varNum, Z, SRCmajority, DSTmajority)
long varNum;
Logical Z;
long SRCmajority;
long DSTmajority;
{
   CDFstatus status; long cPct, sArraysPct;
   long ignoredNum, dataType, numElements, recVary, dimVarys[CDF_MAX_DIMS];
   long numDims, dimSizes[CDF_MAX_DIMS], NvalueBytes, maxRec, blockingFactor;
   long cType, cParms[CDF_MAX_PARMS], sArraysType, sArraysParms[CDF_MAX_PARMS],
	sRecordsType, reservePct;
   char varName[CDF_VAR_NAME_LEN256 + 1]; long recNum, fromRec, toRec;
   int dimN; void *padValue; long nValuesPerRecord, nRecords;
   Byte **handles[2], *buffer1, *buffer2;
   size_t nValueBytes[2]; long nHypers, nValues, hyperN;
   struct GroupStruct groups; struct HyperStruct hyper;
   Logical srcRowMajor = ROWmajor(SRCmajority), switchMajority;
   static char cvtMsg[] = "      converting variable values...    ";
   /***************************************************************************
   * Inquire existing variable in source CDF.
   ***************************************************************************/
   status = CDFlib (SELECT_, CDF_, srcId,
			     VAR(Z), varNum,
		    GET_, VAR_NAME(Z), varName,
			  VAR_DATATYPE(Z), &dataType,
			  VAR_NUMELEMS(Z), &numElements,
			  VAR_RECVARY(Z), &recVary,
			  VAR_DIMVARYS(Z), dimVarys,
			  VAR_MAXREC(Z), &maxRec,
			  VAR_BLOCKINGFACTOR(Z), &blockingFactor,
			  VAR_SPARSERECORDS(Z), &sRecordsType,
			  VAR_SPARSEARRAYS(Z), &sArraysType,
					       sArraysParms,
					       &sArraysPct,
			  VAR_COMPRESSION(Z), &cType, cParms, &cPct,
		    NULL_);
   if (!StatusHandlerCvt("SRC",status)) return FALSE;
   if (cType != NO_COMPRESSION) {
     status = CDFlib (CONFIRM_, VAR_RESERVEPERCENT(Z), &reservePct,
		      NULL_);
     if (!StatusHandlerCvt("SRC",status)) return FALSE;
   }
   if (Z) {
     status = CDFlib (SELECT_, CDF_, srcId,
		      GET_, zVAR_NUMDIMS_, &numDims,
			    zVAR_DIMSIZES_, dimSizes,
		      NULL_);
     if (!StatusHandlerCvt("SRC",status)) return FALSE;
   }
   else {
     status = CDFlib (SELECT_, CDF_, srcId,
		      GET_, rVARs_NUMDIMS_, &numDims,
			    rVARs_DIMSIZES_, dimSizes,
		      NULL_);
     if (!StatusHandlerCvt("SRC",status)) return FALSE;
   }
   /***************************************************************************
   * Creating new variable in destination CDF.
   ***************************************************************************/
   if (mLog) {
     WriteOut (stdout, "    Converting ");
     WriteOut (stdout, BOO(Z,"zV","rV"));
     WriteOut (stdout, "ariable \"");
     WriteOut (stdout, varName);
     WriteOut (stdout, "\"\n");
   }
   if (Z) {
     status = CDFlib (SELECT_, CDF_, dstId,
		      CREATE_, zVAR_, varName, dataType, numElements, numDims,
				      dimSizes, recVary, dimVarys, &ignoredNum,
		      NULL_);
     if (!StatusHandlerCvt("DST",status)) return FALSE;
   }
   else {
     status = CDFlib (SELECT_, CDF_, dstId,
		      CREATE_, rVAR_, varName, dataType, numElements, recVary,
				     dimVarys, &ignoredNum,
		      NULL_);
     if (!StatusHandlerCvt("DST",status)) return FALSE;
   }
   /***************************************************************************
   * Specify pad value for new variable (if it exists for old variable).
   ***************************************************************************/
   NvalueBytes = numElements * CDFelemSize(dataType);
   padValue = cdf_AllocateMemory ((size_t) NvalueBytes, FatalError);
   status = CDFlib (SELECT_, CDF_, srcId,
		    GET_, VAR_PADVALUE(Z), padValue,
		    NULL_);
   if (status != NO_PADVALUE_SPECIFIED) {
     if (!StatusHandlerCvt("SRC",status)) return FALSE;
     status = CDFlib (SELECT_, CDF_, dstId,
		      PUT_, VAR_PADVALUE(Z), padValue,
		      NULL_);
     if (!StatusHandlerCvt("DST",status)) return FALSE;
   }
   cdf_FreeMemory (padValue, FatalError);
   /***************************************************************************
   * Determine sparseness/compression/blocking factor.
   ***************************************************************************/
   if (sparseRecords != NULL) {
     struct sRecordsVarStruct *Var;
     if (sparseRecords->VARs.sRecordsType != SOURCEsparseRECORDS) {
       sRecordsType = sparseRecords->VARs.sRecordsType;
     }
     for (Var = sparseRecords->VARhead; Var != NULL; Var = Var->next) {
	if (!strcmpITB(Var->name,varName)) {
	  sRecordsType = Var->sRecordsType;
	  break;
	}
     }
   }
   if (compression != NULL) {
     struct cVarStruct *Var; int i;
     if (compression->VARs.cType != SOURCEcompression) {
       cType = compression->VARs.cType;
       for (i = 0; i < CDF_MAX_PARMS; i++) {
	  cParms[i] = compression->VARs.cParms[i];
       }
       blockingFactor = compression->VARs.bf;
       reservePct = compression->VARs.reserve;
     }
     for (Var = compression->VARhead; Var != NULL; Var = Var->next) {
	if (!strcmpITB(Var->name,varName)) {
	  cType = Var->cType;
	  for (i = 0; i < CDF_MAX_PARMS; i++) cParms[i] = Var->cParms[i];
	  blockingFactor = Var->bf;
	  reservePct = Var->reserve;
	  break;
	}
     }
   }
   if (!recVary) blockingFactor = MINIMUM(blockingFactor,1);
   /***************************************************************************
   * Put compression/sparseness/blocking factor.
   ***************************************************************************/
   status = CDFlib (SELECT_, CDF_, dstId,
		    PUT_, VAR_BLOCKINGFACTOR(Z), blockingFactor,
			  VAR_SPARSERECORDS(Z), sRecordsType,
			  VAR_SPARSEARRAYS(Z), sArraysType, sArraysParms,
			  VAR_COMPRESSION(Z), cType, cParms,
		    NULL_);
   if (!StatusHandlerCvt("DST",status)) return FALSE;
   if (cType != NO_COMPRESSION) {
     status = CDFlib (SELECT_, VAR_RESERVEPERCENT(Z), reservePct,
		      NULL_);
     if (!StatusHandlerCvt("DST",status)) return FALSE;
   }
   CHECKforABORTso
   /***************************************************************************
   * Return (success) if no records exist.
   ***************************************************************************/
   if (maxRec == NO_RECORD) return TRUE;
   /***************************************************************************
   * Read/write values using hyper groups...
   ***************************************************************************/
   for (dimN = 0, nValuesPerRecord = 1; dimN < numDims; dimN++) {
      if (dimVarys[dimN])
	nValuesPerRecord *= dimSizes[dimN];
      else
	dimSizes[dimN] = 1;
   }
   handles[0] = &buffer1;
   handles[1] = &buffer2;
   nValueBytes[0] = (size_t) NvalueBytes;
   nValueBytes[1] = (size_t) NvalueBytes;
   if (mLog) {
     WriteOut (stdout, cvtMsg);
     if (pctLog)
       pctOn = TRUE;
     else
       WriteOut (stdout, "\n");
   }
   switchMajority = (numDims > 1 && SRCmajority != DSTmajority);
   for (recNum = 0; recNum <= maxRec; recNum = toRec + 1) {
      /************************************************************************
      * Determine the next allocated record.
      ************************************************************************/
      status = CDFlib (SELECT_, CDF_, srcId,
		       GET_, VAR_ALLOCATEDFROM(Z), recNum, &fromRec,
		       NULL_);
      if (!StatusHandlerCvt("SRC",status)) return FALSE;
      /************************************************************************
      * Determine the last allocated record (before the next unallocated one).
      * Do not let this exceed the maximum record written to the variable.
      ************************************************************************/
      status = CDFlib (SELECT_, CDF_, srcId,
		       GET_, VAR_ALLOCATEDTO(Z), fromRec, &toRec,
		       NULL_);
      if (!StatusHandlerCvt("SRC",status)) return FALSE;
      toRec = MINIMUM(toRec,maxRec);
      /************************************************************************
      * Allocate the records unless the variable is compressed or has sparse
      * arrays.
      ************************************************************************/
      if (cType == NO_COMPRESSION && sArraysType == NO_SPARSEARRAYS) {
	status = CDFlib (SELECT_, CDF_, dstId,
			 PUT_, VAR_ALLOCATEBLOCK(Z), fromRec, toRec,
			 NULL_);
	if (!StatusHandlerCvt("DST",status)) return FALSE;
      }
      /************************************************************************
      * Calculate the number of records in this group.
      ************************************************************************/
      nRecords = toRec - fromRec + 1;
      /************************************************************************
      * If the majority is being switched...
      ************************************************************************/
      if (switchMajority) {
	AllocateBuffers (nRecords, numDims, dimSizes, &groups, 0, 2, handles,
			 nValueBytes, srcRowMajor, MINnHYPERS, FatalError);
	if (HyperFullRecord(&groups,numDims)) {
	  long nBytesPerRecord = nValuesPerRecord * NvalueBytes, recX;
	  InitHyperParms (&hyper, &groups, numDims, &nHypers, &nValues);
	  hyper.recNumber = fromRec;
	  for (hyperN = 0; hyperN < nHypers; hyperN++) {
	     status = HYPER_READ (srcId, Z, hyper, buffer1);
	     if (!StatusHandlerCvt("SRC",status)) return FALSE;
	     if (pctLog) {
	       if (!pctOn) WriteOut (stdout, cvtMsg);
	       WriteOutPct (PCTx(PCT(hyperN,nHypers,1,3),toRec,maxRec));
	       pctOn = TRUE;
	     }
	     CHECKforABORTso
	     for (recX = 0; recX < hyper.recCount; recX++) {
	        size_t offset = (size_t) (recX * nBytesPerRecord);
	        if (srcRowMajor)
	          ROWtoCOL (buffer1 + offset, buffer2 + offset, numDims,
		            dimSizes, NvalueBytes);
	        else
	          COLtoROW (buffer1 + offset, buffer2 + offset, numDims,
		            dimSizes, NvalueBytes);
	     }
	     if (pctLog) {
	       if (!pctOn) WriteOut (stdout, cvtMsg);
	       WriteOutPct (PCTx(PCT(hyperN,nHypers,2,3),toRec,maxRec));
	       pctOn = TRUE;
	     }
	     CHECKforABORTso
	     status = HYPER_WRITE (dstId, Z, hyper, buffer2);
	     if (!StatusHandlerCvt("DST",status)) return FALSE;
	     if (pctLog) {
	       if (!pctOn) WriteOut (stdout, cvtMsg);
	       WriteOutPct (PCTx(PCT(hyperN,nHypers,3,3),toRec,maxRec));
	       pctOn = TRUE;
	     }
	     CHECKforABORTso
	     IncrHyperParms (&hyper, &groups, numDims, srcRowMajor, &nValues);
          }
          cdf_FreeMemory (buffer1, FatalError);
          cdf_FreeMemory (buffer2, FatalError);
        }
	else {
          cdf_FreeMemory (buffer2, FatalError);
          InitHyperParms (&hyper, &groups, numDims, &nHypers, &nValues);
	  hyper.recNumber = fromRec;
          for (hyperN = 0; hyperN < nHypers; hyperN++) {
             long indices[CDF_MAX_DIMS]; Byte *value = buffer1; long valueN;
             status = HYPER_READ (srcId, Z, hyper, buffer1);
             if (!StatusHandlerCvt("SRC",status)) return FALSE;
             CHECKforABORTso
             if (pctLog) {
	       if (!pctOn) WriteOut (stdout, cvtMsg);
	       WriteOutPct (PCTx(PCT(hyperN,nHypers,1,2),toRec,maxRec));
	       pctOn = TRUE;
             }
             status = CDFlib (SELECT_,CDF_,dstId,
				      BOO(Z,zVAR_RECNUMBER_,
				            rVARs_RECNUMBER_),hyper.recNumber,
		              NULL_);
             if (!StatusHandlerCvt("DST",status)) return FALSE;
             for (dimN = 0; dimN < numDims; dimN++) {
	        indices[dimN] = hyper.dimIndices[dimN];
             }
             for (valueN = 0; valueN < nValues; valueN++) {
	        status = CDFlib (SELECT_, BOO(Z,zVAR_DIMINDICES_,
					        rVARs_DIMINDICES_), indices,
			         PUT_, VAR_DATA(Z), value,
			         NULL_);
	        if (!StatusHandlerCvt("DST",status)) return FALSE;
	        if (srcRowMajor)
	          INCRindicesROW (numDims, dimSizes, indices);
	        else
	          INCRindicesCOL (numDims, dimSizes, indices);
	        value += (size_t) NvalueBytes;
             }
             if (pctLog) {
	       if (!pctOn) WriteOut (stdout, cvtMsg);
	       WriteOutPct (PCTx(PCT(hyperN,nHypers,2,2),toRec,maxRec));
	       pctOn = TRUE;
             }
             IncrHyperParms (&hyper, &groups, numDims, srcRowMajor, &nValues);
             CHECKforABORTso
          }
          cdf_FreeMemory (buffer1, FatalError);
	}
      }
      else {
	AllocateBuffers (nRecords, numDims, dimSizes, &groups, 0, 1, handles,
			 nValueBytes, srcRowMajor, MINnHYPERS, FatalError);
	InitHyperParms (&hyper, &groups, numDims, &nHypers, &nValues);
	hyper.recNumber = fromRec;
	for (hyperN = 0; hyperN < nHypers; hyperN++) {
	   status = HYPER_READ (srcId, Z, hyper, buffer1);
	   if (!StatusHandlerCvt("SRC",status)) return FALSE;
	   if (pctLog) {
	     if (!pctOn) WriteOut (stdout, cvtMsg);
	     WriteOutPct (PCTx(PCT(hyperN,nHypers,1,2),toRec,maxRec));
	     pctOn = TRUE;
	   }
	   CHECKforABORTso
	   status = HYPER_WRITE (dstId, Z, hyper, buffer1);
	   if (!StatusHandlerCvt("DST",status)) return FALSE;
	   if (pctLog) {
	     if (!pctOn) WriteOut (stdout, cvtMsg);
	     WriteOutPct (PCTx(PCT(hyperN,nHypers,2,2),toRec,maxRec));
	     pctOn = TRUE;
	   }
	   CHECKforABORTso
	   IncrHyperParms (&hyper, &groups, numDims, srcRowMajor, &nValues);
	}
	cdf_FreeMemory (buffer1, FatalError);
      }
   }
   if (pctLog) {
     WriteOut (stdout, "\n");
     pctOn = FALSE;
   }
   return TRUE;
}

/******************************************************************************
* ParseSparsenessOptions.
******************************************************************************/

#define BADs(s) FreeSparseness(s); return NULL;

struct SparseRecordsStruct *ParseSparsenessOptions (options)
char *options;
{
  struct SparseRecordsStruct *S; size_t nBytes, len; Logical done;
  char *p1, *p2; struct sRecordsVarStruct *Var;
  /****************************************************************************
  * Allocate/initialize sparseness structure.
  ****************************************************************************/
  nBytes = sizeof(struct SparseRecordsStruct);
  S = (struct SparseRecordsStruct *) cdf_AllocateMemory (nBytes, FatalError);
  S->VARs.sRecordsType = SOURCEsparseRECORDS;
  S->VARhead = NULL;
  /****************************************************************************
  * Scan options...
  * NOTE: This duplication (see `ParseCompressionOptions') could probably be
  * eliminated.
  ****************************************************************************/
  switch (*options) {
    case '(':           /* VMS-style. */
      p1 = options + 1;
      len = strlen(p1);
      if (len == 0) { BADs(S) }
      if (p1[len-1] != ')') { BADs(S) }
      p1[len-1] = NUL;
      break;
    case '"':		/* UNIX-style on a Mac (double quotes not stripped). */
      p1 = options + 1;
      len = strlen(p1);
      if (len == 0) { BADs(S) }
      if (p1[len-1] != '"') { BADs(S) }
      p1[len-1] = NUL;
      break;
    default:            /* UNIX-style on a UNIX machine or IBM PC. */
      p1 = options;
      break;
  }
  for (;;) {
     p2 = strchr (p1, ':');
     if (p2 == NULL) { BADs(S) }
     *p2 = NUL;
     if (strcmpIgCase(p1,"vars") != 0) {
       p1 = p2 + 1;
       if (!ParseSparsenessToken(&p1,&p2,&(S->VARs.sRecordsType),&done)) {
	 BADs(S)
       }
       if (done) return S;
       p1 = p2 + 1;
       continue;
     }
     if (strcmpIgCase(p1,"var") != 0) {
       nBytes = sizeof(struct sRecordsVarStruct);
       Var = (struct sRecordsVarStruct *) cdf_AllocateMemory (nBytes, FatalError);
       Var->name = NULL;
       Var->next = S->VARhead;
       S->VARhead = Var;
       p1 = p2 + 1;
       if (*p1 == NUL) { BADs(S) }
       p2 = strchr (p1 + 1, *p1);
       if (p2 == NULL) { BADs(S) }
       p1 += 1;
       *p2 = NUL;
       len = strlen (p1);
       if (len < 1) { BADs(S) }
       Var->name = (char *) cdf_AllocateMemory (len, FatalError);
       strcpyX (Var->name, p1, len);
       p2 += 1;
       if (*p2 != ':') { BADs(S) }
       p1 = p2 + 1;
       if (!ParseSparsenessToken(&p1,&p2,&(Var->sRecordsType),&done)) {
	 BADs(S)
       }
       if (done) return S;
       p1 = p2 + 1;
       continue;
     }
     BADs(S)
  }
}

/******************************************************************************
* ParseSparsenessToken.
******************************************************************************/

Logical ParseSparsenessToken (p1, p2, sRecordsType, done)
char **p1;
char **p2;
long *sRecordsType;
Logical *done;
{
  char *sToken;
  if (**p1 == NUL) return FALSE;
  *p2 = strchr (*p1, ',');
  if (*p2 == NULL) {
    *done = TRUE;
  }
  else {
    *done = FALSE;
    **p2 = NUL;
  }
  sToken = *p1;
  if (strcmpIgCase(sToken,"sRecords.PAD") != 0) {
    *sRecordsType = PAD_SPARSERECORDS;
    return TRUE;
  }
  if (strcmpIgCase(sToken,"sRecords.PREV") != 0) {
    *sRecordsType = PREV_SPARSERECORDS;
    return TRUE;
  }
  if (strcmpIgCase(sToken,"sRecords.NO") != 0) {
    *sRecordsType = NO_SPARSERECORDS;
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************
* ParseCompressionOptions.
******************************************************************************/

#define BADc(c) FreeCompression(c); return NULL;

struct CompressionStruct *ParseCompressionOptions (options)
char *options;
{
  struct CompressionStruct *C; size_t nBytes, len;
  Logical done; char *p1, *p2; struct cVarStruct *Var;
  /****************************************************************************
  * Allocate/initialize compression structure.
  ****************************************************************************/
  nBytes = sizeof(struct CompressionStruct);
  C = (struct CompressionStruct *) cdf_AllocateMemory (nBytes, FatalError);
  C->CDF.cType = SOURCEcompression;
  C->VARs.cType = SOURCEcompression;
  C->VARhead = NULL;
  /****************************************************************************
  * Scan options...
  ****************************************************************************/
  switch (*options) {
    case '(':           /* VMS-style. */
      p1 = options + 1;
      len = strlen(p1);
      if (len == 0) { BADc(C) }
      if (p1[len-1] != ')') { BADc(C) }
      p1[len-1] = NUL;
      break;
    case '"':		/* UNIX-style on a Mac (double quotes not stripped). */
      p1 = options + 1;
      len = strlen(p1);
      if (len == 0) { BADc(C) }
      if (p1[len-1] != '"') { BADc(C) }
      p1[len-1] = NUL;
      break;
    default:            /* UNIX-style on a UNIX machine or IBM PC. */
      p1 = options;
      break;
  }
  for (;;) {
     p2 = strchr (p1, ':');
     if (p2 == NULL) { BADc(C) }
     *p2 = NUL;
     if (strcmpIgCase(p1,"cdf") != 0) {
       p1 = p2 + 1;
       if (!ParseCompressionTokenAndBF(&p1,&p2,
				       &(C->CDF.cType),
				       C->CDF.cParms,
				       NULL,NULL,&done)) { BADc(C) }
       if (done) return C;
       p1 = p2 + 1;
       continue;
     }
     if (strcmpIgCase(p1,"vars") != 0) {
       p1 = p2 + 1;
       if (!ParseCompressionTokenAndBF(&p1,&p2,
				       &(C->VARs.cType),
				       C->VARs.cParms,
				       &(C->VARs.bf),
				       &(C->VARs.reserve),
				       &done)) { BADc(C) }
       if (done) return C;
       p1 = p2 + 1;
       continue;
     }
     if (strcmpIgCase(p1,"var") != 0) {
       nBytes = sizeof(struct cVarStruct);
       Var = (struct cVarStruct *) cdf_AllocateMemory (nBytes, FatalError);
       Var->name = NULL;
       Var->next = C->VARhead;
       C->VARhead = Var;
       p1 = p2 + 1;
       if (*p1 == NUL) { BADc(C) }
       p2 = strchr (p1 + 1, *p1);
       if (p2 == NULL) { BADc(C) }
       p1 += 1;
       *p2 = NUL;
       len = strlen (p1);
       if (len < 1) { BADc(C) }
       Var->name = (char *) cdf_AllocateMemory (len, FatalError);
       strcpyX (Var->name, p1, len);
       p2 += 1;
       if (*p2 != ':') { BADc(C) }
       p1 = p2 + 1;
       if (!ParseCompressionTokenAndBF(&p1,&p2,
				       &(Var->cType),
				       Var->cParms,
				       &(Var->bf),
				       &(Var->reserve),
				       &done)) { BADc(C) }
       if (done) return C;
       p1 = p2 + 1;
       continue;
     }
     BADc(C)
  }
}

/******************************************************************************
* ParseCompressionTokenAndBF.
******************************************************************************/

Logical ParseCompressionTokenAndBF (p1, p2, cType, cParms, bf, reserve, done)
char **p1;
char **p2;
long *cType;
long cParms[CDF_MAX_PARMS];
long *bf;
long *reserve;
Logical *done;
{
  int count, level; long v1, v2; char *cToken, *p;
  if (**p1 == NUL) return FALSE;
  *p2 = strchr (*p1, ',');
  if (*p2 == NULL) {
    *done = TRUE;
  }
  else {
    *done = FALSE;
    **p2 = NUL;
  }
  cToken = *p1;
  p = strchr (*p1, ':');
  if (p == NULL)
    count = 0;
  else {
    *p = NUL;
    count = sscanf (p+1, "%ld:%ld", &v1, &v2);
    if (count < 1) return FALSE;
  }
  if (strcmpIgCase(cToken,"none") != 0) {
    if (count > 0) return FALSE;
    *cType = NO_COMPRESSION;
    ASSIGNnotNULL (bf, 0)
    ASSIGNnotNULL (reserve, 0)
    return TRUE;
  }
  if (strcmpIgCase(cToken,"rle.0") != 0) {
    *cType = RLE_COMPRESSION;
    cParms[0] = RLE_OF_ZEROs;
    ASSIGNnotNULL (bf, BOO(count > 0,v1,0))
    ASSIGNnotNULL (reserve, BOO(count > 1,v2,0))
    return TRUE;
  }
  if (strcmpIgCase(cToken,"huff.0") != 0) {
    *cType = HUFF_COMPRESSION;
    cParms[0] = OPTIMAL_ENCODING_TREES;
    ASSIGNnotNULL (bf, BOO(count > 0,v1,0))
    ASSIGNnotNULL (reserve, BOO(count > 1,v2,0))
    return TRUE;
  }
  if (strcmpIgCase(cToken,"ahuff.0") != 0) {
    *cType = AHUFF_COMPRESSION;
    cParms[0] = OPTIMAL_ENCODING_TREES;
    ASSIGNnotNULL (bf, BOO(count > 0,v1,0))
    ASSIGNnotNULL (reserve, BOO(count > 1,v2,0))
    return TRUE;
  }
  for (level = 1; level <= 9; level++) {
     char gzipToken[MAX_GZIP_TOKEN_LEN+1];
     sprintf (gzipToken, "gzip.%d", level);
     if (strcmpIgCase(cToken,gzipToken) != 0) {
       *cType = GZIP_COMPRESSION;
       cParms[0] = level;
       ASSIGNnotNULL (bf, BOO(count > 0,v1,0))
       ASSIGNnotNULL (reserve, BOO(count > 1,v2,0))
       return TRUE;
     }
  }
/*
  if (strcmpIgCase(cToken,"zlib.0") != 0) {
    *cType = ZLIB_COMPRESSION;
    cParms[0] = ZLIB_OF_ZEROs;
    ASSIGNnotNULL (bf, BOO(count > 0,v1,0))
    ASSIGNnotNULL (reserve, BOO(count > 1,v2,0))
    return TRUE;
  }
*/
  return FALSE;
}

/******************************************************************************
* FreeSparseness.
******************************************************************************/

void FreeSparseness (sparseRecords)
struct SparseRecordsStruct *sparseRecords;
{
  struct sRecordsVarStruct *Var = sparseRecords->VARhead;
  while (Var != NULL) {
    struct sRecordsVarStruct *VARnext = Var->next;
    if (Var->name != NULL) cdf_FreeMemory (Var->name, FatalError);
    cdf_FreeMemory (Var, FatalError);
    Var = VARnext;
  }
  cdf_FreeMemory (sparseRecords, FatalError);
}

/******************************************************************************
* FreeCompression.
******************************************************************************/

void FreeCompression (compression)
struct CompressionStruct *compression;
{
  struct cVarStruct *Var = compression->VARhead;
  while (Var != NULL) {
    struct cVarStruct *VARnext = Var->next;
    if (Var->name != NULL) cdf_FreeMemory (Var->name, FatalError);
    cdf_FreeMemory (Var, FatalError);
    Var = VARnext;
  }
  cdf_FreeMemory (compression, FatalError);
}

/******************************************************************************
* StatusHandlerCvt.
******************************************************************************/

Logical StatusHandlerCvt (which, status)
char *which;
CDFstatus status;
{
  char text[CDF_STATUSTEXT_LEN+1];            /* Explanation text. */
  char oText[MAX_OUTPUT_TEXT_LEN+1];          /* Output text. */
  if (StatusERROR(status)) {
    if (report[ERRORs]) {
      if (pctOn) {
	WriteOut (stdout, "\n");	      /* Move to start of next line. */
	pctOn = FALSE;
      }
      CDFlib (SELECT_, CDF_STATUS_, status,
	      GET_, STATUS_TEXT_, text,
	      NULL_);
      sprintf (oText, "ERROR,%s> %s", which, text);
      OutputWithMargin (stdout, oText, MAX_SCREENLINE_LEN, 0);
    }
    return FALSE;
  }
  if (StatusWARN(status)) {
    if (report[WARNs]) {
      if (pctOn) {
	WriteOut (stdout, "\n");	      /* Move to start of next line. */
	pctOn = FALSE;
      }
      CDFlib (SELECT_, CDF_STATUS_, status,
	      GET_, STATUS_TEXT_, text,
	      NULL_);
      sprintf (oText, "WARNING,%s> %s", which, text);
      OutputWithMargin (stdout, oText, MAX_SCREENLINE_LEN, 0);
    }
    return TRUE;
  }
  if (StatusINFO(status)) {
    if (report[INFOs]) {
      if (pctOn) {
	WriteOut (stdout, "\n");	      /* Move to start of next line. */
	pctOn = FALSE;
      }
      CDFlib (SELECT_, CDF_STATUS_, status,
	      GET_, STATUS_TEXT_, text,
	      NULL_);
      sprintf (oText, "INFO,%s> %s", which, text);
      OutputWithMargin (stdout, oText, MAX_SCREENLINE_LEN, 0);
    }
    return TRUE;
  }
  return TRUE;          /* CDF_OK */
}

/******************************************************************************
* ConvertQOPs.
*    Returns TRUE if execution should continue.
******************************************************************************/

#if defined(mac)
Logical ConvertQOPs (argC, argV)
int *argC;
char **argV[];
{
  DialogPtr dialogP;
  DialogRecord dRecord;
  WindowPtr behind = (WindowPtr) -1;
  ControlHandle controlHs[MAXIMUMin+1];
  Rect iRect;
#ifdef __MWERKS__
  ModalFilterUPP FilterDialogQOPsoUPP = NULL;
  FileFilterUPP FilterForCDFsUPP = NULL;
  UserItemUPP OutlineDefaultButtonUPP;
#endif
  short itemN, iType, i;
  static Logical first = TRUE;
  char cancelTitle[] = "Cancel";

  static Logical logMsg = DEFAULTlogCVT;
  static Logical dispPct = DEFAULTpctCVT;
  static Logical deleteDst = DEFAULTdelCVT;
  static Logical pageOutput = DEFAULTpageCVT;
  static Logical reportInfos = REPORTinfosDEFAULT;
  static Logical reportWarns = REPORTwarningsDEFAULT;
  static Logical reportErrors = REPORTerrorsDEFAULT;
  static Logical negToPos = DEFAULT_NEGtoPOSfp0;
  static Logical dispStats = DEFAULTstatsCVT;
  static int dstFormat = 0;
  static int dstMajority = 0;
  static int zModeSrc = DEFAULTzModeCVT;
  static int dstEncoding = 0;
  static Str255 cacheText = "\p";
  static Str255 srcText = "\p";
  static Str255 dstText = "\p";
  static Str255 sktText = "\p";
  static Str255 sparseText = "\p";
  static Str255 compressText = "\p";

  /****************************************************************************
  * Create the dialog and get the control handles.
  ****************************************************************************/

  dialogP = GetNewDialog (QOPri, &dRecord, behind);
  
  for (itemN = 1; itemN <= MAXIMUMin; itemN++) {
     GetDItem (dialogP, itemN, &iType, (Handle *) &controlHs[itemN], &iRect);
  }

  /****************************************************************************
  * Set the control values.
  ****************************************************************************/

  SetIText ((Handle) controlHs[SRCTEXTin], srcText);
  SetIText ((Handle) controlHs[DSTTEXTin], dstText);
  SetIText ((Handle) controlHs[SKTTEXTin], sktText);
  SetIText ((Handle) controlHs[CACHEin], cacheText);
  SetIText ((Handle) controlHs[SPARSEin], sparseText);
  SetIText ((Handle) controlHs[COMPRESSin], compressText);

  if (logMsg) SetCtlValue (controlHs[LOGin], 1);
  if (dispPct) SetCtlValue (controlHs[PCTin], 1);
  if (deleteDst) SetCtlValue (controlHs[DELETEin], 1);
  if (pageOutput) SetCtlValue (controlHs[PAGEin], 1);
  if (reportInfos) SetCtlValue (controlHs[INFOin], 1);
  if (reportWarns) SetCtlValue (controlHs[WARNin], 1);
  if (reportErrors) SetCtlValue (controlHs[ERRORin], 1);
  if (negToPos) SetCtlValue (controlHs[NEGZin], 1);
  if (dispStats) SetCtlValue (controlHs[STATSin], 1);

  SetCtlValue (controlHs[DSTFORMATinBASE+dstFormat], 1);
  SetCtlValue (controlHs[DSTMAJORITYinBASE+dstMajority], 1);
  SetCtlValue (controlHs[SRCzMODEinBASE+zModeSrc], 1);
  SetCtlValue (controlHs[DSTENCODINGinBASE+dstEncoding], 1);

#ifndef __MWERKS__
  SetDItem (dialogP, (short) ODBin, (short) userItem,
	    (Handle) OutlineDefaultButton, &iRect);
#else
  OutlineDefaultButtonUPP = NewUserItemProc (OutlineDefaultButton);
  SetDItem (dialogP, (short) ODBin, (short) userItem,
	    (Handle) OutlineDefaultButtonUPP, &iRect);
#endif
  /****************************************************************************
  * Change the "Quit" button to a "Cancel" button after the first time.
  ****************************************************************************/

  if (first)
    first = FALSE;
  else
    SetCTitle (controlHs[CANCELin], CtoPstr(cancelTitle));

  /****************************************************************************
  * Display the dialog and wait for user actions.
  ****************************************************************************/
    
  ShowWindow ((WindowPtr) dialogP);
  SetCursor (ARROW_CURSOR);
#ifdef __MWERKS__
  FilterDialogQOPsoUPP = NewModalFilterProc((ProcPtr) FilterDialogQOPso);
#endif

  for (;;) {
#ifndef __MWERKS__
    ModalDialog (FilterDialogQOPso, &itemN);
#else
    ModalDialog (FilterDialogQOPsoUPP, &itemN);
#endif
    switch (itemN) {
      /************************************************************************
      * Ok.
      ************************************************************************/
      case OKin: {
		char *formatOptions[] = { NULL, "-single", "-multi" };
		char *majorityOptions[] = { NULL, "-row", "-column" };
		char *encodingSymbols[] = { NULL, "host", "sun", "vax", "sgi",
					      "hp", "ibmpc", "ibmrs", "network",
					      "mac", "decstation", "next", "alphavmsd",
					      "alphavmsg", "alphaosf1", "alphavmsi" };
		int n;
		char tempS1[1+1];

		/**********************************************************************
		* Get the value of each control.
		**********************************************************************/

		GetIText ((Handle) controlHs[SRCTEXTin], srcText);
		GetIText ((Handle) controlHs[DSTTEXTin], dstText);
		GetIText ((Handle) controlHs[SKTTEXTin], sktText);
		GetIText ((Handle) controlHs[CACHEin], cacheText);
		GetIText ((Handle) controlHs[SPARSEin], sparseText);
		GetIText ((Handle) controlHs[COMPRESSin], compressText);

		logMsg = GetCtlValue (controlHs[LOGin]);
		dispPct = GetCtlValue (controlHs[PCTin]);
		deleteDst = GetCtlValue (controlHs[DELETEin]);
		pageOutput = GetCtlValue (controlHs[PAGEin]);
		reportInfos = GetCtlValue (controlHs[INFOin]);
		reportWarns = GetCtlValue (controlHs[WARNin]);
		reportErrors = GetCtlValue (controlHs[ERRORin]);
		negToPos = GetCtlValue (controlHs[NEGZin]);
		dispStats = GetCtlValue (controlHs[STATSin]);
	
		for (dstFormat = 0; dstFormat < 3; dstFormat++) {
		   if (GetCtlValue(controlHs[DSTFORMATinBASE+dstFormat])) break;
		}
		for (dstMajority = 0; dstMajority < 3; dstMajority++) {
		   if (GetCtlValue(controlHs[DSTMAJORITYinBASE+dstMajority])) break;
		}
		for (dstEncoding = 0; dstEncoding < 16; dstEncoding++) {
		   if (GetCtlValue(controlHs[DSTENCODINGinBASE+dstEncoding])) break;
		}
		for (zModeSrc = 0; zModeSrc < 3; zModeSrc++) {
		   if (GetCtlValue(controlHs[SRCzMODEinBASE+zModeSrc])) break;
		}
	
		/**********************************************************************
		* Build argc/argv.
		**********************************************************************/

		*argC = 11 + BOO(NULpString(srcText),0,1) +
				     BOO(NULpString(dstText),0,1) +
				     BOO(NULpString(sktText),0,2) +
				     BOO(NULpString(cacheText),0,2) +
				     BOO(NULpString(sparseText),0,2) +
				     BOO(NULpString(compressText),0,2) +
				     BOO(dstFormat != 0, 1, 0) +
				     BOO(dstMajority != 0, 1, 0) +
				     BOO(dstEncoding != 0, 2, 0);
		*argV = (char **) cdf_AllocateMemory (*argC * sizeof(char *),
										  FatalError);
	
		n = 0;
		MAKEstrARGv (argV, n, pgmName)

		if (!NULpString(srcText)) {
		  PtoCstr (srcText);
		  MAKEstrARGv (argV, n, (char *) srcText)
		  CtoPstr ((char *) srcText);
		}

		if (!NULpString(dstText)) {
		  PtoCstr (dstText);
		  MAKEstrARGv (argV, n, (char *) dstText)
		  CtoPstr ((char *) dstText);
		}

		MAKEbooARGv (argV, n, logMsg, "-log", "-nolog")
		MAKEbooARGv (argV, n, dispPct, "-percent", "-nopercent")
		MAKEbooARGv (argV, n, deleteDst, "-delete", "-nodelete")
		MAKEbooARGv (argV, n, pageOutput, "-page", "-nopage")
		MAKEbooARGv (argV, n, negToPos, "-neg2posfp0", "-noneg2posfp0")
		MAKEbooARGv (argV, n, dispStats, "-statistics", "-nostatistics")
	
		MAKEstrARGv (argV, n, "-zmode")
		sprintf (tempS1, "%d", zModeSrc);
		MAKEstrARGv (argV, n, tempS1)

		MAKEstrARGv (argV, n, "-report")
		MAKEstrARGv (argV, n, StatusCodeReportOptions(reportErrors,
							      reportWarns,
							      reportInfos))

		if (dstFormat != 0) {
		  MAKEstrARGv (argV, n, formatOptions[dstFormat])
		}

		if (dstMajority != 0) {
		  MAKEstrARGv (argV, n, majorityOptions[dstMajority])
		}

		if (dstEncoding != 0) {
		  MAKEstrARGv (argV, n, "-encoding")
		  MAKEstrARGv (argV, n, encodingSymbols[dstEncoding])
		}

		if (!NULpString(sktText)) {
		  MAKEstrARGv (argV, n, "-skeleton")
		  PtoCstr (sktText);
		  MAKEstrARGv (argV, n, (char *) sktText)
		  CtoPstr ((char *) sktText);
		}

		if (!NULpString(cacheText)) {
		  MAKEstrARGv (argV, n, "-cache")
		  PtoCstr (cacheText);
		  MAKEstrARGv (argV, n, (char *) cacheText)
		  CtoPstr ((char *) cacheText);
		}

		if (!NULpString(sparseText)) {
		  MAKEstrARGv (argV, n, "-sparseness")
		  PtoCstr (sparseText);
		  MAKEstrARGv (argV, n, (char *) sparseText)
		  CtoPstr ((char *) sparseText);
		}

		if (!NULpString(compressText)) {
		  MAKEstrARGv (argV, n, "-compression")
		  PtoCstr (compressText);
		  MAKEstrARGv (argV, n, (char *) compressText)
		  CtoPstr ((char *) compressText);
		}

		/**********************************************************************
		* Close the dialog and return.
		**********************************************************************/
#ifdef __MWERKS__
        DisposeRoutineDescriptor (FilterDialogQOPsoUPP);
		DisposeRoutineDescriptor (OutlineDefaultButtonUPP);
#endif
		CloseDialog (dialogP);
		return TRUE;
      }
      /************************************************************************
      * Help.
      ************************************************************************/
      case HELPin: {
		int n;
		*argC = 1;
		*argV = (char **) cdf_AllocateMemory (*argC * sizeof(char *),
										  FatalError);
		n = 0;
		MAKEstrARGv (argV, n, pgmName)
#ifdef __MWERKS__
        DisposeRoutineDescriptor (FilterDialogQOPsoUPP);
		DisposeRoutineDescriptor (OutlineDefaultButtonUPP);
#endif
		CloseDialog (dialogP);
		return TRUE;
      }
      /************************************************************************
      * Cancel.
      ************************************************************************/
      case CANCELin:
#ifdef __MWERKS__
        DisposeRoutineDescriptor (FilterDialogQOPsoUPP);
		DisposeRoutineDescriptor (OutlineDefaultButtonUPP);
#endif
		CloseDialog (dialogP);
		return FALSE;
      /************************************************************************
      * Select source CDF specification.
      ************************************************************************/
      case SRCSELECTin: {
		StandardFileReply srcReply;
		char srcPath[DU_MAX_PATH_LEN+1];
#ifndef __MWERKS__
		StandardGetFile (FilterForCDFs, -1, NULL, &srcReply);
#else
		FilterForCDFsUPP = NewFileFilterProc((ProcPtr) FilterForCDFs);
		StandardGetFile (FilterForCDFsUPP, -1, NULL, &srcReply);
        DisposeRoutineDescriptor (FilterForCDFsUPP);
#endif
		if (srcReply.sfGood && !srcReply.sfIsFolder && !srcReply.sfIsVolume) {
		  BuildMacPath (&srcReply.sfFile, srcPath, TRUE);
		  srcText[0] = strlen (srcPath);
		  strcpyX ((char *) &srcText[1], srcPath, 255);
		  SetIText ((Handle) controlHs[SRCTEXTin], srcText);
		}
		break;
      }
      /************************************************************************
      * Select destination CDF specification.
      *     The cursor is set because `StandardPutFile' leaves the cursor as
      * an iBeam (instead of returning it to what it was).
      ************************************************************************/
      case DSTSELECTin: {
		StandardFileReply dstReply;
		char dstPath[DU_MAX_PATH_LEN+1], prompt[] = "Enter specification:";
		StandardPutFile (CtoPstr(prompt), CtoPstr(""), &dstReply);
		if (dstReply.sfGood && !dstReply.sfIsFolder && !dstReply.sfIsVolume) {
		  BuildMacPath (&dstReply.sfFile, dstPath, TRUE);
		  dstText[0] = strlen (dstPath);
		  strcpyX ((char *) &dstText[1], dstPath, 255);
		  SetIText ((Handle) controlHs[DSTTEXTin], dstText);
		}
		SetCursor (&(qd.arrow));
		break;
      }
      /************************************************************************
      * Select skeleton CDF.
      ************************************************************************/
      case SKTSELECTin: {
		StandardFileReply sktReply;
		char sktPath[DU_MAX_PATH_LEN+1];
#ifndef __MWERKS__
		StandardGetFile (FilterForCDFs, -1, NULL, &sktReply);
#else
		FilterForCDFsUPP = NewFileFilterProc((ProcPtr) FilterForCDFs);
		StandardGetFile (FilterForCDFsUPP, -1, NULL, &sktReply);
        DisposeRoutineDescriptor (FilterForCDFsUPP);
#endif
		if (sktReply.sfGood && !sktReply.sfIsFolder && !sktReply.sfIsVolume) {
		  BuildMacPath (&sktReply.sfFile, sktPath, TRUE);
		  sktText[0] = strlen (sktPath);
		  strcpyX ((char *) &sktText[1], sktPath, 255);
		  SetIText ((Handle) controlHs[SKTTEXTin], sktText);
		}
		break;
      }
      /************************************************************************
      * Check boxes.
      ************************************************************************/
      case LOGin:
		if (GetCtlValue(controlHs[LOGin])) {
		  SetCtlValue (controlHs[LOGin], 0);
		  SetCtlValue (controlHs[PCTin], 0);
		}
		else
		  SetCtlValue (controlHs[LOGin], 1);
		break;
 	  case PCTin:
		if (GetCtlValue(controlHs[PCTin]))
		  SetCtlValue (controlHs[PCTin], 0);
		else {
		  SetCtlValue (controlHs[PCTin], 1);
		  SetCtlValue (controlHs[LOGin], 1);
		}
		break;
      case DELETEin:
      case PAGEin:
      case INFOin:
      case WARNin:
      case ERRORin:
      case NEGZin:
      case STATSin:
		SetCtlValue (controlHs[itemN], BOO(GetCtlValue(controlHs[itemN]),0,1));
		break;
      /************************************************************************
      * Radio buttons.
      ************************************************************************/
      case DSTFORMATinBASE+0:
      case DSTFORMATinBASE+1:
      case DSTFORMATinBASE+2:
		for (i = 0; i < 3; i++) SetCtlValue (controlHs[DSTFORMATinBASE+i], 0);
		SetCtlValue (controlHs[itemN], 1);
		break;
      case DSTMAJORITYinBASE+0:
      case DSTMAJORITYinBASE+1:
      case DSTMAJORITYinBASE+2:
		for (i = 0; i < 3; i++) SetCtlValue (controlHs[DSTMAJORITYinBASE+i],0);
		SetCtlValue (controlHs[itemN], 1);
		break;
      case DSTENCODINGinBASE+0:
      case DSTENCODINGinBASE+1:
      case DSTENCODINGinBASE+2:
      case DSTENCODINGinBASE+3:
      case DSTENCODINGinBASE+4:
      case DSTENCODINGinBASE+5:
      case DSTENCODINGinBASE+6:
      case DSTENCODINGinBASE+7:
      case DSTENCODINGinBASE+8:
      case DSTENCODINGinBASE+9:
      case DSTENCODINGinBASE+10:
      case DSTENCODINGinBASE+11:
      case DSTENCODINGinBASE+12:
      case DSTENCODINGinBASE+13:
      case DSTENCODINGinBASE+14:
      case DSTENCODINGinBASE+15:
		for (i = 0; i < 16; i++) SetCtlValue(controlHs[DSTENCODINGinBASE+i],0);
		SetCtlValue (controlHs[itemN], 1);
		break;
      case SRCzMODEinBASE+0:
      case SRCzMODEinBASE+1:
      case SRCzMODEinBASE+2:
		for (i = 0; i < 3; i++) SetCtlValue (controlHs[SRCzMODEinBASE+i], 0);
		SetCtlValue (controlHs[itemN], 1);
		break;
    }
  }
}
#endif
