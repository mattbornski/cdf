/******************************************************************************
*
*  NSSDC/CDF                                    Header file for CDFmerge.
*
*  Version 1.0, 23-Jul-06, Raytheon.
*
*  Modification history:
*
*   V1.0  23-Jul-06, J Liu      Original version.
*
******************************************************************************/

#if !defined(CDFMERGEh_INCLUDEd__)
#define CDFMERGEh_INCLUDEd__

/******************************************************************************
* Include files.
******************************************************************************/

#include "cdftools.h"
#include "time.h"

/******************************************************************************
* Parameter/qualifier constants.
******************************************************************************/
#define PREFIXMAXLEN   20
#define MAX_MERGELINE_LEN   256

#define PREFIXESqual    0
#define NOPREFIXqual    1
#define ABOUTqual       2
#define LOGqual         3
#define NOLOGqual       4
#define FILEqual        5
#define DATAONLYqual    6
#define NODATAONLYqual  7
#define CDAWEBqual      8
#define NOCDAWEBqual    9
#define MASTERqual     10
#define NOMASTERqual   11
#define AUGMENTLABELqual   12
#define NOAUGMENTLABELqual 13

/******************************************************************************
* Global variables.
******************************************************************************/
static char **prefixes = NULL;

/******************************************************************************
* Function Prototypes.
******************************************************************************/

Logical MergeCDFs (int, char *[]);
void ModifyName (char *, int, char **, int, char *);
void QuitCDF (char *, CDFstatus, char *);
void ParseStringForVariables (int, char *[], int, int, char *[],
                              char *, char *, Logical *);
void ConstructOutputFileName (char *);
void ConstructOutputFileName2 (char *);
void GetOutputFileName (char *);
void GetOutputFileName2 (char *);
void GetDataTimeStamp (char *);
void BreakFields (char *, int, char **, int);
void LoadgAttributes (CDFid);
void LoadgAttributes2 ();
Logical LoadMasterTextFile (char *);
Logical LoadMasterTextFile2 (char *);

/*****************************************************************************/

#endif
