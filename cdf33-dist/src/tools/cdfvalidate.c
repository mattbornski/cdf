/******************************************************************************
*
*  NSSDC/CDF                                  Validate CDF internal records.
*
*  Version 1.0, 15-Jul-08, Peort Systems.
*
*  Modification history:
*
*   V1.0  15-Jul-08, M Liu      Original version.
*
******************************************************************************/

#define CDFVALIDATE
#include "cdfvalidate.h"

/******************************************************************************
* Local macro definitions.
******************************************************************************/

/******************************************************************************
* Main. 
******************************************************************************/

#if !defined(win32) || (defined(win32) && defined(ALONE))
MAIN {
  Logical success = TRUE;
  strcpyX (pgmName, "CDFvalidate", MAX_PROGRAM_NAME_LEN);
  success = ValidateCDFs (argc, argv);
  return BOO(success,EXIT_SUCCESS_,EXIT_FAILURE_);
}
#endif 

/******************************************************************************
* ValidateCDFs.
*    Returns FALSE if an error occurred.
******************************************************************************/

Logical ValidateCDFs (argC, argV)
int argC;
char *argV[];
{
  int i, numParms;
  char **CDFspec;
  QOP *qop; Logical qopError = FALSE;
  static char *validQuals[] = {
    "debug", "about", "validate", "novalidate", NULL
  };
  static int optRequired[] = {
    FALSE, FALSE, FALSE, FALSE, 0
  };
  char oText[MAX_oTEXT_LEN+1];
  char text[CDF_STATUSTEXT_LEN+1];
  Logical debug = FALSE;
  CDFid id;
  CDFstatus status;

  /****************************************************************************
  * Determine qualifiers/options/parameters.
  ****************************************************************************/
  switch (argC) {
    case 1:
      PageOLH ("cdfvalidate.olh", argV[0]);
      return TRUE;
    case 2:
      if (strcmp(argV[1],"-java") == 0) {
        pagingOn = FALSE;
        PageOLH ("cdfvalidatej.olh", argV[0]);
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
      * Check for the `format' qualifier.
      ************************************************************************/
      qopError = qopError |! TFqualifier(qop,&useValidate,VALIDATEqual,
                                         NOVALIDATEqual,
                                         DEFAULTvalidate, "validate");
      /************************************************************************
      * Check for `debug' qualifier.
      ************************************************************************/
      if (qop->qualEntered[DEBUGqual]) {
        debug = TRUE;
      }
      /************************************************************************
      * Get CDF path.
      ************************************************************************/
      if (qop->Nparms < 1) {
        DisplayError ("Missing parameter.");
        qopError = TRUE;
      }
      else {
	numParms = qop->Nparms;
	CDFspec = (char **) malloc(sizeof(char **) * qop->Nparms);
	for (i = 0; i < qop->Nparms; ++i) {
          CDFspec[i] = (char *) malloc(DU_MAX_PATH_LEN + 1);
          strcpyX (CDFspec[i], qop->parms[i], DU_MAX_PATH_LEN);
        }
      }
      /************************************************************************
      * Free QOP memory and check for an error.
      ************************************************************************/
      cdf_FreeMemory (qop, FatalError);
      if (qopError) return FALSE;
      break;
  }
  if (useValidate)
    CDFsetValidate (VALIDATEFILEon);
  else
    CDFsetValidate (VALIDATEFILEoff);
#if !defined(vms)
#if !defined(SOLARIS)
  if (debug) setenv ("CDF_VALIDATE_DEBUG", "yes", 1);
#else
  if (debug) putenv ("CDF_VALIDATE_DEBUG=yes");
#endif
#else
  if (debug) setenv ("CDF$VALIDATE$DEBUG", "yes", 1);
#endif
  for (i = 0; i < numParms; ++i) {
    /**************************************************************************
    * Display `validating' message.
    **************************************************************************/
    strcpyX (oText, "Validating \"", 0);
    strcatX (oText, CDFspec[i], 0);
    if (!EndsWithIgCase(CDFspec[i], ".cdf"))
      strcatX (oText, ".cdf", 0);
    strcatX (oText, "\"...", 0);
    OutputWithMargin (stdout, oText, MAX_SCREENLINE_LEN, 0);
    /**************************************************************************
    * Open CDF file, which will validate the file.
    **************************************************************************/
    status = CDFlib (OPEN_, CDF_, CDFspec[i], &id,
                     NULL_);
    if (status < CDF_OK) {
      CDFlib (SELECT_, CDF_STATUS_, status,
              GET_, STATUS_TEXT_, text,
              NULL_);
      printf ("%s\n", text);
    } else {
      status = CDFlib (CLOSE_, CDF_,
                       NULL_);
      if (status < CDF_OK) {
        CDFlib (SELECT_, CDF_STATUS_, status,
                GET_, STATUS_TEXT_, text,
                NULL_);
        printf ("%s\n", text);
      } 
    }
    free (CDFspec[i]);
    printf("\n");
  }
  free (CDFspec);
  return TRUE; 
}

