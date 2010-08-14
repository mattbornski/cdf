/******************************************************************************
*
*  NSSDC/CDF                                            Memory Functions.
*
*  Version 1.2b, 29-Oct-97, Hughes STX.
*
*  Modification history:
*
*   V1.0  17-May-95, J Love     Original version.
*                    J Williams
*   V1.1  25-Jul-95, J Love     Simple virtual memory function.
*   V1.1a  6-Sep-95, J Love     Added `nBytes' to MEMLOG_.
*   V1.1b 22-Sep-95, J Love     Fixed call to `FreeVMemory'.
*   V1.2  16-Aug-96, J Love	CDF V2.6.
*   V1.2a 28-Oct-96, J Love	Added `return' statement to cdf_FreeMemory for
*				when building for IDL.
*   V1.2b 29-Oct-97, J Love	More Windows NT.
*   V2.0  19-May-05, M Liu      Added "cdf_" to FreeMemory, AllocateMemory, 
*                               ReallocateMemory functions.
*
******************************************************************************/

#define CDFMEM
#include "cdflib.h"
#include "cdflib64.h"

#define MEMLOG		0
#define TO_STDOUT	0

/******************************************************************************
* MEM structures/typedef's/global variables.
******************************************************************************/

#if !defined(BUILDINGforIDL)
typedef struct memSTRUCT {	/* Structure. */
  void *ptr;
  struct memSTRUCT *next;
  size_t nBytes;
} MEM;
typedef MEM *MEMp;		/* Pointer (to structure). */
static MEMp memHeadP = NULL;	/* Head of memory linked list. */
#endif

/******************************************************************************
* Local function prototypes.
******************************************************************************/

#if defined(MICROSOFTC_700) && INCLUDEvMEMORY
static Logical InitVMemory PROTOARGs((void));
static void TermVMemory PROTOARGs((void));
#endif

#if MEMLOG
static uLong TotalBytes PROTOARGs((MEMp memHeadP));
#endif

/******************************************************************************
* Debugging macros.
******************************************************************************/

#if MEMLOG
#if TO_STDOUT
#define ALLOCFAILED(type,which,nBytes) \
  {FILE *fp = FOPEN("mem.log","a"); \
  fprintf (fp, "%sllocation FAILED [%d]: %lu bytes\n", \
	   type, which, (uLong) nBytes); \
  fprintf (stdout, "%sllocation FAILED [%d]: %lu bytes\n", \
	   type, which, (uLong) nBytes); \
  fclose (fp);}
#else
#define ALLOCFAILED(type,which,nBytes) \
  {FILE *fp = FOPEN("mem.log","a"); \
  fprintf (fp, "%sllocation FAILED [%d]: %lu bytes\n", \
	   type, which, (uLong) nBytes); \
  fclose (fp);}
#endif
#else
#define ALLOCFAILED(type,which,nBytes) \

#endif

#if MEMLOG
#if TO_STDOUT
#define ALLOCSUCCESS(type,nBytes,ptr,totalBytes) \
  {FILE *fp = FOPEN("mem.log","a"); uLong totalBytesSave = totalBytes; \
  fprintf (fp, "%sllocated: %lu bytes at %p (%lu bytes total)\n", \
	   type, (uLong) nBytes, ptr, totalBytesSave); \
  fprintf (stdout, "%sllocated: %lu bytes at %p (%lu bytes total)\n", \
	   type, (uLong) nBytes, ptr, totalBytesSave); \
  fclose (fp);}
#else
#define ALLOCSUCCESS(type,nBytes,ptr,totalBytes) \
  {FILE *fp = FOPEN("mem.log","a"); \
  fprintf (fp, "%sllocated: %lu bytes at %p (%lu bytes total)\n", \
	   type, (uLong) nBytes, ptr, totalBytes); \
  fclose (fp);}
#endif
#else
#define ALLOCSUCCESS(type,nBytes,ptr,totalBytes) \

#endif

#if MEMLOG
#if TO_STDOUT
#define FREESUCCESS(type,ptr,nBytes) \
  {FILE *fp = FOPEN("mem.log","a"); \
  fprintf (fp, "Freed (%s): %p, %lu bytes\n", type, ptr, (uLong) nBytes); \
  fprintf (stdout, "Freed (%s): %p, %lu bytes\n", type, ptr, (uLong) nBytes); \
  fclose (fp);}
#else
#define FREESUCCESS(type,ptr,nBytes) \
  {FILE *fp = FOPEN("mem.log","a"); \
  fprintf (fp, "Freed (%s): %p, %lu bytes\n", type, ptr, (uLong) nBytes); \
  fclose (fp);}
#endif
#else
#define FREESUCCESS(type,ptr,nBytes) \

#endif

#if MEMLOG
#if TO_STDOUT
#define FREEFAILED(ptr) \
  {FILE *fp = FOPEN("mem.log","a"); \
  fprintf (fp, "Free failed [pointer not found]: %p\n", ptr); \
  fprintf (stdout, "Free failed [pointer not found]: %p\n", ptr); \
  fclose (fp);}
#else
#define FREEFAILED(ptr) \
  {FILE *fp = FOPEN("mem.log","a"); \
  fprintf (fp, "Free failed [pointer not found]: %p\n", ptr); \
  fclose (fp);}
#endif
#else
#define FREEFAILED(ptr) \

#endif

/******************************************************************************
* cdf_AllocateMemory.
******************************************************************************/

VISIBLE_PREFIX void *cdf_AllocateMemory (nBytes, fatalFnc)
size_t nBytes;
void (*fatalFnc) PROTOARGs((char *msg));
{
#if defined(BUILDINGforIDL)
  return malloc(nBytes);
#else
  MEMp mem;
  if (nBytes < 1) {
    ALLOCFAILED("A",1,nBytes)
    return NULL;
  }
  mem = (MEMp) malloc (sizeof(MEM));
  if (mem == NULL) {
    ALLOCFAILED("A",2,nBytes)
    if (fatalFnc != NULL) (*fatalFnc)("Unable to allocate memory buffer [1].");
    return NULL;
  }
  mem->ptr = (void *) malloc (nBytes);
  if (mem->ptr == NULL) {
    ALLOCFAILED("A",3,nBytes)
    free (mem);
    if (fatalFnc != NULL) (*fatalFnc)("Unable to allocate memory buffer [2].");
    return NULL;
  }
  mem->nBytes = nBytes;
  mem->next = memHeadP;
  memHeadP = mem;
  ALLOCSUCCESS("A",mem->nBytes,mem->ptr,TotalBytes(memHeadP))
  return mem->ptr;
#endif
}

/******************************************************************************
* cdf_ReallocateMemory.
******************************************************************************/

VISIBLE_PREFIX void *cdf_ReallocateMemory (ptr, nBytes, fatalFnc)
void *ptr;
size_t nBytes;
void (*fatalFnc) PROTOARGs((char *msg));
{
#if defined(BUILDINGforIDL)
  return realloc(ptr,nBytes);
#else
  MEMp mem = memHeadP;
  while (mem != NULL) {
    if (mem->ptr == ptr) {
      void *newPtr = (void *) realloc (ptr, nBytes);
      if (newPtr == NULL) {
	ALLOCFAILED("Rea",1,nBytes)
	if (fatalFnc != NULL) {
	  (*fatalFnc)("Unable to reallocate memory buffer [1].");
	}
	return NULL;
      }
      mem->ptr = newPtr;
      mem->nBytes = nBytes;
      ALLOCSUCCESS("Rea",mem->nBytes,mem->ptr,TotalBytes(memHeadP))
      return newPtr;
    }
    mem = mem->next;
  }
  ALLOCFAILED("Rea",2,nBytes)
  if (fatalFnc != NULL) (*fatalFnc)("Unable to reallocate memory buffer [2].");
  return NULL;
#endif
}

/******************************************************************************
* cdf_FreeMemory.
* If NULL is passed as the pointer to free, then free the entire list of
* allocated memory blocks.
******************************************************************************/

VISIBLE_PREFIX int cdf_FreeMemory (ptr, fatalFnc)
void *ptr;
void (*fatalFnc) PROTOARGs((char *msg));
{
#if defined(BUILDINGforIDL)
  free (ptr);
  return 1;
#else
  if (ptr == NULL) {
    int count = 0;
    MEMp mem = memHeadP;
    while (mem != NULL) {
      MEMp memX = mem;
      mem = mem->next;
      FREESUCCESS("NULL",memX->ptr,memX->nBytes)
      free (memX->ptr);
      free (memX);
      count++;
    }
    memHeadP = NULL;
    return count;
  }
  else {
    MEMp mem = memHeadP, memPrev = NULL;
    while (mem != NULL) {
      if (mem->ptr == ptr) {
	MEMp memX = mem;
	if (memPrev == NULL)
	  memHeadP = mem->next;
	else
	  memPrev->next = mem->next;
        FREESUCCESS("one",memX->ptr,memX->nBytes)
	free (memX->ptr);
	free (memX);
	return 1;
      }
      memPrev = mem;
      mem = mem->next;
    }
    FREEFAILED(ptr)
    if (fatalFnc != NULL) (*fatalFnc)("Unable to free memory buffer.");
    return 0;
  }
#endif
}

/******************************************************************************
* cdf_FreeMemoryX.
* If NULL is passed as the pointer to free, then free the entire list of
* allocated memory blocks.
******************************************************************************/

VISIBLE_PREFIX int cdf_FreeMemoryX (ptr, fatalFnc, loc)
void *ptr;
void (*fatalFnc) PROTOARGs((char *msg));
int loc;
{
#if defined(BUILDINGforIDL)
  free (ptr);
  return 1;
#else
  if (ptr == NULL) {
    int count = 0;
    MEMp mem = memHeadP;
    while (mem != NULL) {
      MEMp memX = mem;
      mem = mem->next;
      FREESUCCESS("NULL",memX->ptr,memX->nBytes)
      free (memX->ptr);
      free (memX);
      count++;
    }
    memHeadP = NULL;
    return count;
  }
  else {
    MEMp mem = memHeadP, memPrev = NULL;
    while (mem != NULL) {
      if (mem->ptr == ptr) {
	MEMp memX = mem;
	if (memPrev == NULL)
	  memHeadP = mem->next;
	else
	  memPrev->next = mem->next;
        FREESUCCESS("one",memX->ptr,memX->nBytes)
	free (memX->ptr);
	free (memX);
	return 1;
      }
      memPrev = mem;
      mem = mem->next;
    }
    FREEFAILED(ptr)
    if (fatalFnc != NULL) {
      char tmp[40];
      strcpy (tmp, "Unable to free memory buffer at ");
      sprintf (EofS(tmp), "%d", loc);
      (*fatalFnc)(tmp);
    }
    return 0;
  }
#endif
}

/******************************************************************************
* CallocateMemory.
* Using `calloc' might be more efficient on some platforms if the memory can
* be cleared faster than the loop used here.
******************************************************************************/

VISIBLE_PREFIX void *CallocateMemory (nObjects, objSize, fatalFnc)
size_t nObjects;
size_t objSize;
void (*fatalFnc) PROTOARGs((char *msg));
{
#if defined(BUILDINGforIDL)
  return calloc(nObjects,objSize);
#else
  size_t nBytes = nObjects * objSize, i;
  void *ptr = cdf_AllocateMemory (nBytes, fatalFnc);
  if (ptr != NULL) for (i = 0; i < nBytes; i++) ((Byte *) ptr)[i] = 0;
  return ptr;
#endif
}

/******************************************************************************
* InitVMemory.
*   NOTE: It is a mystery as to why values of `maxParagraphs' other than 128
* (or near 128) cause allocation/loading failures (as well as other nasty
* things).  Be a hero, solve the mystery...
******************************************************************************/

#if defined(MICROSOFTC_700) && INCLUDEvMEMORY
static Logical InitVMemory () {
  uInt maxParagraphs = (uInt) (vMemSize / 16);
  if (!_vheapinit(0U,maxParagraphs,vMemMask)) return FALSE;
  return TRUE;
}
#endif

/******************************************************************************
* TermVMemory.
******************************************************************************/

#if defined(MICROSOFTC_700) && INCLUDEvMEMORY
static void TermVMemory () {
  _vheapterm();
}
#endif

/******************************************************************************
* AllocateVMemory.
******************************************************************************/

#if defined(MICROSOFTC_700) && INCLUDEvMEMORY
STATICforIDL MemHandle AllocateVMemory (nBytes)
size_t nBytes;
{
  MemHandle handle;
  static int first = TRUE;
  if (first) {
    first = FALSE;
    if (!InitVMemory()) return NULL;
    _fatexit (TermVMemory);
  }
  if ((handle = _vmalloc(nBytes)) == _VM_NULL) return NULL;
  return handle;
}
#endif

/******************************************************************************
* LoadVMemory.
******************************************************************************/

#if defined(MICROSOFTC_700) && INCLUDEvMEMORY
STATICforIDL void *LoadVMemory (handle, writeFlag)
MemHandle handle;
Logical writeFlag;      /* TRUE if virtual memory will be written to. */
{
  void *buffer = _vload(handle,BOO(writeFlag,_VM_DIRTY,_VM_CLEAN));
  if (buffer == NULL) {
    FreeVMemory (handle);
    return NULL;
  }
  return buffer;
}
#endif

/******************************************************************************
* FreeVMemory.
******************************************************************************/

#if defined(MICROSOFTC_700) && INCLUDEvMEMORY
STATICforIDL int FreeVMemory (handle)
MemHandle handle;
{
  _vfree (handle);
  return 1;
}
#endif

/******************************************************************************
* TotalBytes.
******************************************************************************/

#if MEMLOG
static uLong TotalBytes (memHeadP)
MEMp memHeadP;
{
  MEMp mem = memHeadP; uLong totalBytes = 0;
  while (mem != NULL) {
    totalBytes += (uLong) mem->nBytes;
    mem = mem->next;
  }
  return totalBytes;
}
#endif
