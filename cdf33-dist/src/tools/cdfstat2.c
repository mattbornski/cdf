/******************************************************************************
*
*  NSSDC/CDF						Display statistics.
*
*  Version 1.7, 1-Jul-96, Hughes STX.
*
*  Modification history:
*
*   V1.0  29-Aug-91, J Love	Original version (for CDF V2.1).
*   V1.1  15-Nov-91, J Love	Changes for port to IBM-RS6000 (AIX)
*				involving signed vs. unsigned 'char', etc.
*   V1.2  19-Mar-92, J Love	IBM PC port.  Added filter fills option.
*   V1.3  12-Jun-92, J Love	CDF V2.3 (shareable/NeXT/zVar).
*   V1.4  20-Oct-93, J Love	CDF V2.4.
*   V1.5  12-Dec-94, J Love	CDF V2.5.
*   V1.5a  9-May-95, J Love	EPOCH styles.
*   V1.6  27-Jul-95, J Love	Hyper groups.
*   V1.7   1-Jul-96, J Love	CDF V2.6.
*
******************************************************************************/

#include "cdfstats.h"

/******************************************************************************
* DISPstat.
******************************************************************************/

void DISPstat (Var)
struct VarStruct *Var;
{
  char line[MAX_SCREENLINE_LEN+1];

  DisplayMin (Var);
  DisplayMax (Var);
  DisplayFill (Var);

  strcpyX (line, "       monotonic: ", MAX_SCREENLINE_LEN);
  if (Var->checkMonotonicVar)
    if (Var->monoInited)
      switch (Var->monoState) {
        case _Init:
          strcatX (line, "Steady (one value)", MAX_SCREENLINE_LEN);
          break;
        case _Steady:
          strcatX (line, "Steady (all values the same)", MAX_SCREENLINE_LEN);
          break;
        case _Increase:
          strcatX (line, "Increase", MAX_SCREENLINE_LEN);
          break;
        case _Decrease:
          strcatX (line, "Decrease", MAX_SCREENLINE_LEN);
          break;
        case _noIncrease:
          strcatX (line, "noIncrease (some values the same)",
		   MAX_SCREENLINE_LEN);
          break;
        case _noDecrease:
          strcatX (line, "noDecrease (some values the same)",
		   MAX_SCREENLINE_LEN);
          break;
        case _False:
          strcatX (line, "False", MAX_SCREENLINE_LEN);
          break;
      }
    else
      strcatX (line, "n/a (all fill values)", MAX_SCREENLINE_LEN);
  else
    strcatX (line, "n/a", MAX_SCREENLINE_LEN);
  strcatX (line, "\n\n", MAX_SCREENLINE_LEN);
  WriteOut (OUTfp, line);

  return;
}

/******************************************************************************
* DisplayMin.
******************************************************************************/

void DisplayMin (Var)
struct VarStruct *Var;
{ 
  char line[MAX_SCREENLINE_LEN+1];

  strcpyX (line, "         minimum: ", MAX_SCREENLINE_LEN);
  if (Var->minmaxInited)
    EncodeValuesFormat (Var->dataTypeV, Var->numElemsV, Var->min,
		        &line[strlen(line)], Var->format, 0,
		        MAX_SCREENLINE_LEN - strlen(line), EPOCH0_STYLE);
  else
    strcatX (line, "n/a (all fill values)", MAX_SCREENLINE_LEN);
  strcatX (line, "\n", MAX_SCREENLINE_LEN);
  WriteOut (OUTfp, line);

  if (Var->rangeCheckVar) {
    strcpyX (line, "    min in range: ", MAX_SCREENLINE_LEN);
    if (Var->minmaxInited)
      if (Var->oneINrange)
        EncodeValuesFormat (Var->dataTypeV, Var->numElemsV, Var->minINrange,
			    &line[strlen(line)], Var->format, 0,
			    MAX_SCREENLINE_LEN - strlen(line), EPOCH0_STYLE);
      else
        strcatX (line, "(none)", MAX_SCREENLINE_LEN);
    else
      strcatX (line, "n/a (all fill values)", MAX_SCREENLINE_LEN);
    strcatX (line, "\n", MAX_SCREENLINE_LEN);
    WriteOut (OUTfp, line);

    strcpyX (line, "       valid min: ", MAX_SCREENLINE_LEN);
    EncodeValuesFormat (Var->dataTypeV, Var->numElemsV, Var->validmin,
		        &line[strlen(line)], Var->format, 0,
		        MAX_SCREENLINE_LEN - strlen(line), EPOCH0_STYLE);
    if (Var->minmaxInited) sprintf (&line[strlen(line)], ", %ld low value%s",
				    Var->low, (Var->low == 1 ? "" : "s"));
    strcatX (line, "\n\n", MAX_SCREENLINE_LEN);
    WriteOut (OUTfp, line);
  }
  return;
}

/******************************************************************************
* DisplayMax.
******************************************************************************/

void DisplayMax (Var)
struct VarStruct *Var;
{ 
  char line[MAX_SCREENLINE_LEN+1];

  strcpyX (line, "         maximum: ", MAX_SCREENLINE_LEN);
  if (Var->minmaxInited)
    EncodeValuesFormat (Var->dataTypeV, Var->numElemsV, Var->max,
		        &line[strlen(line)], Var->format, 0,
		        MAX_SCREENLINE_LEN - strlen(line), EPOCH0_STYLE);
  else
    strcatX (line, "n/a (all fill values)", MAX_SCREENLINE_LEN);
  strcatX (line, "\n", MAX_SCREENLINE_LEN);
  WriteOut (OUTfp, line);

  if (Var->rangeCheckVar) {
    strcpyX (line, "    max in range: ", MAX_SCREENLINE_LEN);
    if (Var->minmaxInited)
      if (Var->oneINrange)
        EncodeValuesFormat (Var->dataTypeV, Var->numElemsV, Var->maxINrange,
			    &line[strlen(line)], Var->format, 0,
			    MAX_SCREENLINE_LEN - strlen(line), EPOCH0_STYLE);
      else
        strcatX (line, "(none)", MAX_SCREENLINE_LEN);
    else
      strcatX (line, "n/a (all fill values)", MAX_SCREENLINE_LEN);
    strcatX (line, "\n", MAX_SCREENLINE_LEN);
    WriteOut (OUTfp, line);

    strcpyX (line, "       valid max: ", MAX_SCREENLINE_LEN);
    EncodeValuesFormat (Var->dataTypeV, Var->numElemsV, Var->validmax,
		        &line[strlen(line)], Var->format, 0,
		        MAX_SCREENLINE_LEN - strlen(line), EPOCH0_STYLE);
    if (Var->minmaxInited) sprintf (&line[strlen(line)], ", %ld high value%s",
				    Var->high, (Var->high == 1 ? "" : "s"));
    strcatX (line, "\n\n", MAX_SCREENLINE_LEN);
    WriteOut (OUTfp, line);
  }
  return;
}

/******************************************************************************
* DisplayFill.
******************************************************************************/

void DisplayFill (Var)
struct VarStruct *Var;
{
  char line[MAX_SCREENLINE_LEN+1];

  if (Var->ignoreFillsVar) {
    strcpyX (line, "      fill value: ", MAX_SCREENLINE_LEN);
    EncodeValuesFormat (Var->dataTypeV, Var->numElemsV, Var->fillval,
		        &line[strlen(line)], /*Var->format*/ NULL, 0,
		        MAX_SCREENLINE_LEN - strlen(line), EPOCH0_STYLE);
    sprintf (&line[strlen(line)], ", %ld fill value%s",
	     Var->fills, (Var->fills == 1 ? "" : "s"));
    strcatX (line, "\n\n", MAX_SCREENLINE_LEN);
    WriteOut (OUTfp, line);
  }
  return;
}
