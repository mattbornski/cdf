/******************************************************************************
*
*  NSSDC/CDF					Header file for cdfvalidate.
*
*  Version 1.0, 20-Aug-08, Perot System.
*
*  Modification history:
*
*   V1.0  20-Aug-08, M Liu 	Original version.
*
******************************************************************************/

#if !defined(CDFValidateh_INCLUDEd__)
#define CDFValidateh_INCLUDEd__

/******************************************************************************
* Include files.
******************************************************************************/

#include "cdflib.h"
#include "cdflib64.h"
#include "cdftools.h"

/******************************************************************************
* QOP constants.
******************************************************************************/

#define DEBUGqual		0
#define ABOUTqual		1
#define VALIDATEqual            2
#define NOVALIDATEqual          3

/******************************************************************************
* Global variables.
******************************************************************************/
#if defined(CDFVALIDATE)
Logical useValidate = DEFAULTvalidate;
#else
extern Logical useValidate;
#endif

/******************************************************************************
* Function Prototypes.
******************************************************************************/

Logical ValidateCDFs PROTOARGs((int argC, char *argV[]));

/*****************************************************************************/

#endif
