#ifndef CDFINTERNALMODULE_H
#define CDFINTERNALMODULE_H

/**
 * Include requisisite headers for this module.
 **/
/* The Python module creation header. */
#include "Python.h"
/* Defines macros for the CDF library.  Macros provide what CDF considers
 * to be its "standard interface".  The macros refer to an instance of a
 * CDFlib object, which implements what CDF considers to be its "internal
 * interface".  The internal interface and its dependencies are implemented
 * by a collection of C files in the src/ directory, which the Python
 * extension must be aware of.  These are listed out in setup.py.  At this
 * level, however, we do not need to know about them, as the Python
 * distutils should take care of linking against them when compiling the
 * extension.  Only the interface, as defined in cdf.h, is necessary here. */
#include "cdf.h"
/* Used for standardized types.  When working with numeric values, field
 * length is very important. */
/* not currently used, but could be used to reference int8_t, int32_t, etc.
#include <stdint.h>*/



/* Simple macro for maximum length to allocate for strings. */
#define MAX_OUTPUT_STRING_LEN CDF_COPYRIGHT_LEN + 1

/**
 * Helpful macros for repetitive tasks.
 **/
/* Declare constant. */
#define DECLARE_CONSTANT(constant) /*\
    static PyObject *cdf_##constant;*/
/* Define constant. */
#define DEFINE_AND_EXPORT_CONSTANT(constant) \
    PyModule_AddIntConstant(module, #constant, constant);
/*    cdf_##constant = PyLong_FromLong(constant); \
    Py_INCREF(cdf_##constant); \
    PyModule_AddObject(module, #constant, cdf_##constant);*/
/* Declare error. */
#define DECLARE_ERROR(error)
#define DEFINE_AND_EXPORT_ERROR(error)



/**
 * Declare objects for this module.
 **/
/**
 * Declare common objects for this module.
 **/
/* Error messages.  These will be exported as Python exceptions, and their
 * values will be accessible to the C code. */
DECLARE_ERROR(ATTR_NAME_TRUNC)
DECLARE_ERROR(CDF_NAME_TRUNC)
DECLARE_ERROR(VAR_NAME_TRUNC)
DECLARE_ERROR(NEGATIVE_FP_ZERO)
DECLARE_ERROR(FUTURE_CDF)
DECLARE_ERROR(ATTR_EXISTS)
DECLARE_ERROR(BAD_CDF_ID)
DECLARE_ERROR(BAD_DATA_TYPE)
DECLARE_ERROR(BAD_DIM_SIZE)
DECLARE_ERROR(BAD_DIM_INDEX)
DECLARE_ERROR(BAD_ENCODING)
DECLARE_ERROR(BAD_MAJORITY)
DECLARE_ERROR(BAD_NUM_DIMS)
DECLARE_ERROR(BAD_REC_NUM)
DECLARE_ERROR(BAD_SCOPE)
DECLARE_ERROR(BAD_NUM_ELEMS)
DECLARE_ERROR(CDF_OPEN_ERROR)
DECLARE_ERROR(CDF_EXISTS)
DECLARE_ERROR(BAD_FORMAT)
DECLARE_ERROR(BAD_ALLOCATE_RECS)
DECLARE_ERROR(BAD_CDF_EXTENSION)
DECLARE_ERROR(NO_SUCH_ATTR)
DECLARE_ERROR(NO_SUCH_ENTRY)
DECLARE_ERROR(NO_SUCH_VAR)
DECLARE_ERROR(VAR_READ_ERROR)
DECLARE_ERROR(VAR_WRITE_ERROR)
DECLARE_ERROR(BAD_ARGUMENT)
DECLARE_ERROR(IBM_PC_OVERFLOW)
DECLARE_ERROR(TOO_MANY_VARS)
DECLARE_ERROR(VAR_EXISTS)
DECLARE_ERROR(BAD_MALLOC)
DECLARE_ERROR(NOT_A_CDF)
DECLARE_ERROR(CORRUPTED_V2_CDF)
DECLARE_ERROR(VAR_OPEN_ERROR)
DECLARE_ERROR(BAD_INITIAL_RECS)
DECLARE_ERROR(BAD_EXTEND_RECS)
DECLARE_ERROR(END_OF_VAR)
DECLARE_ERROR(BAD_CDFSTATUS)
DECLARE_ERROR(CDF_INTERNAL_ERROR)
DECLARE_ERROR(BAD_NUM_VARS)
DECLARE_ERROR(BAD_REC_COUNT)
DECLARE_ERROR(BAD_REC_INTERVAL)
DECLARE_ERROR(BAD_DIM_COUNT)
DECLARE_ERROR(BAD_DIM_INTERVAL)
DECLARE_ERROR(BAD_VAR_NUM)
DECLARE_ERROR(BAD_ATTR_NUM)
DECLARE_ERROR(BAD_ENTRY_NUM)
DECLARE_ERROR(BAD_ATTR_NAME)
DECLARE_ERROR(BAD_VAR_NAME)
DECLARE_ERROR(NO_ATTR_SELECTED)
DECLARE_ERROR(NO_ENTRY_SELECTED)
DECLARE_ERROR(NO_VAR_SELECTED)
DECLARE_ERROR(BAD_CDF_NAME)
DECLARE_ERROR(CANNOT_CHANGE)
DECLARE_ERROR(NO_STATUS_SELECTED)
DECLARE_ERROR(NO_CDF_SELECTED)
DECLARE_ERROR(READ_ONLY_DISTRIBUTION)
DECLARE_ERROR(CDF_CLOSE_ERROR)
DECLARE_ERROR(VAR_CLOSE_ERROR)
DECLARE_ERROR(BAD_FNC_OR_ITEM)
DECLARE_ERROR(ILLEGAL_ON_V1_CDF)
DECLARE_ERROR(CDH_OPEN_ERROR)
DECLARE_ERROR(CDH_CLOSE_ERROR)
DECLARE_ERROR(BAD_CACHE_SIZE)
DECLARE_ERROR(CDF_CREATE_ERROR)
DECLARE_ERROR(NO_SUCH_CDF)
DECLARE_ERROR(VAR_CREATE_ERROR)
DECLARE_ERROR(READ_ONLY_MODE)
DECLARE_ERROR(ILLEGAL_IN_zMODE)
DECLARE_ERROR(BAD_zMODE)
DECLARE_ERROR(BAD_READONLY_MODE)
DECLARE_ERROR(CDF_READ_ERROR)
DECLARE_ERROR(CDF_WRITE_ERROR)
DECLARE_ERROR(ILLEGAL_FOR_SCOPE)
DECLARE_ERROR(NO_MORE_ACCESS)
DECLARE_ERROR(BAD_DECODING)
DECLARE_ERROR(BAD_NEGtoPOSfp0_MODE)
DECLARE_ERROR(UNSUPPORTED_OPERATION)
DECLARE_ERROR(NO_WRITE_ACCESS)
DECLARE_ERROR(NO_DELETE_ACCESS)

/* Various constants.  These will be exported as objects.  Their values
 * will be accessible to this C code. */
DECLARE_CONSTANT(SINGLE_FILE)
DECLARE_CONSTANT(MULTI_FILE)

DECLARE_CONSTANT(CDF_BYTE)
DECLARE_CONSTANT(CDF_CHAR)
DECLARE_CONSTANT(CDF_INT1)
DECLARE_CONSTANT(CDF_UCHAR)
DECLARE_CONSTANT(CDF_UINT1)
DECLARE_CONSTANT(CDF_INT2)
DECLARE_CONSTANT(CDF_UINT2)
DECLARE_CONSTANT(CDF_INT4)
DECLARE_CONSTANT(CDF_UINT4)
DECLARE_CONSTANT(CDF_REAL4)
DECLARE_CONSTANT(CDF_FLOAT)
DECLARE_CONSTANT(CDF_REAL8)
DECLARE_CONSTANT(CDF_DOUBLE)
DECLARE_CONSTANT(CDF_EPOCH)
DECLARE_CONSTANT(CDF_EPOCH16)

DECLARE_CONSTANT(HOST_ENCODING)
DECLARE_CONSTANT(NETWORK_ENCODING)
DECLARE_CONSTANT(VAX_ENCODING)
DECLARE_CONSTANT(ALPHAVMSd_ENCODING)
DECLARE_CONSTANT(ALPHAVMSg_ENCODING)
DECLARE_CONSTANT(SUN_ENCODING)
DECLARE_CONSTANT(SGi_ENCODING)
DECLARE_CONSTANT(DECSTATION_ENCODING)
DECLARE_CONSTANT(ALPHAOSF1_ENCODING)
DECLARE_CONSTANT(IBMRS_ENCODING)
DECLARE_CONSTANT(HP_ENCODING)
DECLARE_CONSTANT(IBMPC_ENCODING)
DECLARE_CONSTANT(NeXT_ENCODING)
DECLARE_CONSTANT(MAC_ENCODING)

DECLARE_CONSTANT(HOST_DECODING)
DECLARE_CONSTANT(NETWORK_DECODING)
DECLARE_CONSTANT(VAX_DECODING)
DECLARE_CONSTANT(ALPHAVMSd_DECODING)
DECLARE_CONSTANT(ALPHAVMSg_DECODING)
DECLARE_CONSTANT(SUN_DECODING)
DECLARE_CONSTANT(SGi_DECODING)
DECLARE_CONSTANT(DECSTATION_DECODING)
DECLARE_CONSTANT(ALPHAOSF1_DECODING)
DECLARE_CONSTANT(IBMRS_DECODING)
DECLARE_CONSTANT(HP_DECODING)
DECLARE_CONSTANT(IBMPC_DECODING)
DECLARE_CONSTANT(NeXT_DECODING)
DECLARE_CONSTANT(MAC_DECODING)

DECLARE_CONSTANT(ROW_MAJOR)
DECLARE_CONSTANT(COL_MAJOR)

DECLARE_CONSTANT(VARY)
DECLARE_CONSTANT(NOVARY)

DECLARE_CONSTANT(GLOBAL_SCOPE)
DECLARE_CONSTANT(VARIABLE_SCOPE)

DECLARE_CONSTANT(READONLYon)
DECLARE_CONSTANT(READONLYoff)

DECLARE_CONSTANT(zMODEoff)
DECLARE_CONSTANT(zMODEon1)
DECLARE_CONSTANT(zMODEon2)

DECLARE_CONSTANT(NEGtoPOSfp0on)
DECLARE_CONSTANT(NEGtoPOSfp0off)

DECLARE_CONSTANT(CDF_MAX_DIMS)

DECLARE_CONSTANT(CDF_PATHNAME_LEN)
DECLARE_CONSTANT(CDF_VAR_NAME_LEN)
DECLARE_CONSTANT(CDF_ATTR_NAME_LEN)
DECLARE_CONSTANT(CDF_COPYRIGHT_LEN)
DECLARE_CONSTANT(CDF_STATUSTEXT_LEN)

/* Various constants.  These will be exported as objects.  Their values
 * will be accessible to this C code. */
DECLARE_CONSTANT(CREATE_)
DECLARE_CONSTANT(OPEN_)
DECLARE_CONSTANT(DELETE_)
DECLARE_CONSTANT(CLOSE_)
DECLARE_CONSTANT(SELECT_)
DECLARE_CONSTANT(CONFIRM_)
DECLARE_CONSTANT(GET_)
DECLARE_CONSTANT(PUT_)

DECLARE_CONSTANT(NULL_)

DECLARE_CONSTANT(CDF_)
DECLARE_CONSTANT(CDF_NAME_)
DECLARE_CONSTANT(CDF_ENCODING_)
DECLARE_CONSTANT(CDF_DECODING_)
DECLARE_CONSTANT(CDF_MAJORITY_)
DECLARE_CONSTANT(CDF_FORMAT_)
DECLARE_CONSTANT(CDF_COPYRIGHT_)
DECLARE_CONSTANT(CDF_NUMrVARS_)
DECLARE_CONSTANT(CDF_NUMzVARS_)
DECLARE_CONSTANT(CDF_NUMATTRS_)
DECLARE_CONSTANT(CDF_NUMgATTRS_)
DECLARE_CONSTANT(CDF_NUMvATTRS_)
DECLARE_CONSTANT(CDF_VERSION_)
DECLARE_CONSTANT(CDF_RELEASE_)
DECLARE_CONSTANT(CDF_INCREMENT_)
DECLARE_CONSTANT(CDF_STATUS_)
DECLARE_CONSTANT(CDF_READONLY_MODE_)
DECLARE_CONSTANT(CDF_zMODE_)
DECLARE_CONSTANT(CDF_NEGtoPOSfp0_MODE_)
DECLARE_CONSTANT(LIB_COPYRIGHT_)
DECLARE_CONSTANT(LIB_VERSION_)
DECLARE_CONSTANT(LIB_RELEASE_)
DECLARE_CONSTANT(LIB_INCREMENT_)
DECLARE_CONSTANT(LIB_subINCREMENT_)
DECLARE_CONSTANT(rVARs_NUMDIMS_)
DECLARE_CONSTANT(rVARs_DIMSIZES_)
DECLARE_CONSTANT(rVARs_MAXREC_)
DECLARE_CONSTANT(rVARs_RECDATA_)
DECLARE_CONSTANT(rVARs_RECNUMBER_)
DECLARE_CONSTANT(rVARs_RECCOUNT_)
DECLARE_CONSTANT(rVARs_RECINTERVAL_)
DECLARE_CONSTANT(rVARs_DIMINDICES_)
DECLARE_CONSTANT(rVARs_DIMCOUNTS_)
DECLARE_CONSTANT(rVARs_DIMINTERVALS_)
DECLARE_CONSTANT(rVAR_)
DECLARE_CONSTANT(rVAR_NAME_)
DECLARE_CONSTANT(rVAR_DATATYPE_)
DECLARE_CONSTANT(rVAR_NUMELEMS_)
DECLARE_CONSTANT(rVAR_RECVARY_)
DECLARE_CONSTANT(rVAR_DIMVARYS_)
DECLARE_CONSTANT(rVAR_NUMBER_)
DECLARE_CONSTANT(rVAR_DATA_)
DECLARE_CONSTANT(rVAR_HYPERDATA_)
DECLARE_CONSTANT(rVAR_SEQDATA_)
DECLARE_CONSTANT(rVAR_SEQPOS_)
DECLARE_CONSTANT(rVAR_MAXREC_)
DECLARE_CONSTANT(rVAR_MAXallocREC_)
DECLARE_CONSTANT(rVAR_DATASPEC_)
DECLARE_CONSTANT(rVAR_PADVALUE_)
DECLARE_CONSTANT(rVAR_INITIALRECS_)
DECLARE_CONSTANT(rVAR_EXTENDRECS_)
DECLARE_CONSTANT(rVAR_nINDEXRECORDS_)
DECLARE_CONSTANT(rVAR_nINDEXENTRIES_)
DECLARE_CONSTANT(rVAR_EXISTENCE_)
DECLARE_CONSTANT(zVARs_MAXREC_)
DECLARE_CONSTANT(zVARs_RECDATA_)
DECLARE_CONSTANT(zVAR_)
DECLARE_CONSTANT(zVAR_NAME_)
DECLARE_CONSTANT(zVAR_DATATYPE_)
DECLARE_CONSTANT(zVAR_NUMELEMS_)
DECLARE_CONSTANT(zVAR_NUMDIMS_)
DECLARE_CONSTANT(zVAR_DIMSIZES_)
DECLARE_CONSTANT(zVAR_RECVARY_)
DECLARE_CONSTANT(zVAR_DIMVARYS_)
DECLARE_CONSTANT(zVAR_NUMBER_)
DECLARE_CONSTANT(zVAR_DATA_)
DECLARE_CONSTANT(zVAR_HYPERDATA_)
DECLARE_CONSTANT(zVAR_SEQDATA_)
DECLARE_CONSTANT(zVAR_SEQPOS_)
DECLARE_CONSTANT(zVAR_MAXREC_)
DECLARE_CONSTANT(zVAR_MAXallocREC_)
DECLARE_CONSTANT(zVAR_DATASPEC_)
DECLARE_CONSTANT(zVAR_PADVALUE_)
DECLARE_CONSTANT(zVAR_INITIALRECS_)
DECLARE_CONSTANT(zVAR_EXTENDRECS_)
DECLARE_CONSTANT(zVAR_nINDEXRECORDS_)
DECLARE_CONSTANT(zVAR_nINDEXENTRIES_)
DECLARE_CONSTANT(zVAR_EXISTENCE_)
DECLARE_CONSTANT(zVAR_RECNUMBER_)
DECLARE_CONSTANT(zVAR_RECCOUNT_)
DECLARE_CONSTANT(zVAR_RECINTERVAL_)
DECLARE_CONSTANT(zVAR_DIMINDICES_)
DECLARE_CONSTANT(zVAR_DIMCOUNTS_)
DECLARE_CONSTANT(zVAR_DIMINTERVALS_)
DECLARE_CONSTANT(ATTR_)
DECLARE_CONSTANT(ATTR_SCOPE_)
DECLARE_CONSTANT(ATTR_NAME_)
DECLARE_CONSTANT(ATTR_NUMBER_)
DECLARE_CONSTANT(ATTR_MAXgENTRY_)
DECLARE_CONSTANT(ATTR_NUMgENTRIES_)
DECLARE_CONSTANT(ATTR_MAXrENTRY_)
DECLARE_CONSTANT(ATTR_NUMrENTRIES_)
DECLARE_CONSTANT(ATTR_MAXzENTRY_)
DECLARE_CONSTANT(ATTR_NUMzENTRIES_)
DECLARE_CONSTANT(ATTR_EXISTENCE_)
DECLARE_CONSTANT(gENTRY_)
DECLARE_CONSTANT(gENTRY_EXISTENCE_)
DECLARE_CONSTANT(gENTRY_DATATYPE_)
DECLARE_CONSTANT(gENTRY_NUMELEMS_)
DECLARE_CONSTANT(gENTRY_DATASPEC_)
DECLARE_CONSTANT(gENTRY_DATA_)
DECLARE_CONSTANT(rENTRY_)
DECLARE_CONSTANT(rENTRY_NAME_)
DECLARE_CONSTANT(rENTRY_EXISTENCE_)
DECLARE_CONSTANT(rENTRY_DATATYPE_)
DECLARE_CONSTANT(rENTRY_NUMELEMS_)
DECLARE_CONSTANT(rENTRY_DATASPEC_)
DECLARE_CONSTANT(rENTRY_DATA_)
DECLARE_CONSTANT(zENTRY_)
DECLARE_CONSTANT(zENTRY_NAME_)
DECLARE_CONSTANT(zENTRY_EXISTENCE_)
DECLARE_CONSTANT(zENTRY_DATATYPE_)
DECLARE_CONSTANT(zENTRY_NUMELEMS_)
DECLARE_CONSTANT(zENTRY_DATASPEC_)
DECLARE_CONSTANT(zENTRY_DATA_)
DECLARE_CONSTANT(STATUS_TEXT_)
DECLARE_CONSTANT(CDF_CACHESIZE_)
DECLARE_CONSTANT(rVARs_CACHESIZE_)
DECLARE_CONSTANT(zVARs_CACHESIZE_)
DECLARE_CONSTANT(rVAR_CACHESIZE_)
DECLARE_CONSTANT(zVAR_CACHESIZE_)
DECLARE_CONSTANT(zVARs_RECNUMBER_)
DECLARE_CONSTANT(rVAR_ALLOCATERECS_)
DECLARE_CONSTANT(zVAR_ALLOCATERECS_)
DECLARE_CONSTANT(DATATYPE_SIZE_)
DECLARE_CONSTANT(CURgENTRY_EXISTENCE_)
DECLARE_CONSTANT(CURrENTRY_EXISTENCE_)
DECLARE_CONSTANT(CURzENTRY_EXISTENCE_)

DECLARE_CONSTANT(CDFwithSTATS_)

/**
 * Declare functions.
 **/
long *allocatedArrayFromOwnedPythonSequence(PyObject *list);
PyObject *ownedPythonListFromArray(void *array, long len, long type);
void **multiDimensionalArray(long *dims, long count, long size);
void cleanupMultiDimensionalArray(void **array, long *dims, long count);
PyObject *ownedPythonListOfListsFromArray(void **array, long *dims, long count, long type);



/* Token types. */
typedef struct {
    short valid;
    long token;
    PyObject *(*function)(long, PyObject *, long *);
} CdfFirstTierToken;

typedef struct {
    short valid;
    long token;
    PyObject *(*function)(long, long, PyObject *, long (*)(PyObject *));
    long argumentsRequired;
    long (*helper)(PyObject *);
} CdfSecondTierToken;

/* The highest-level token handling functions. */
PyObject *CdfFirstTierTokenHandler(PyObject *,
    CdfFirstTierToken *);
PyObject *CdfSecondTierTokenHandler(long, PyObject *, long *,
    CdfSecondTierToken *);

/* Token macros. */
#define DECLARE_FIRST_TIER_TOKEN(token) \
    PyObject *cdf_internal_1_##token(\
        long one, PyObject *tokens, long *tokenOffset) { \
        return CdfSecondTierTokenHandler(one, tokens, tokenOffset, \
            cdf_internal_token_tables_##token); \
    }
#define DECLARE_FIRST_TIER_TOKEN_TABLE(token)

#define FIRST_TIER_TOKEN(token) {1, token, &cdf_internal_1_##token},
#define SECOND_TIER_TOKEN(token, input, output) {1, token, &tokenFormat_##input_##output, NULL, NULL},
#define FIRST_TIER_TOKEN_TABLE_TERMINATOR() {1, NULL_, NULL}, {0, 0, NULL}
#define SECOND_TIER_TOKEN_TABLE_TERMINATOR() {0, 0, NULL, 0, NULL}

/* Third level token handling functions. */
/* The naming scheme is as follows:
 *     tokenFormat_<INPUT>_<OUTPUT>()
 *     where INPUT and OUTPUT are sequences of characters representing the
 *     ordering and type of the input and output Python values.
 *     "l" indicates long
 *     "v" indicates value, the exact type of which needs to be determined
 *     "s" indicates string
 *     "p" indicates pointer
 *     "x" indicates nothing of that type
 *
 *     A capital letter indicates a list of that type.
 */
PyObject *tokenFormat_x_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_l_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_L_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_p_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_s_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_ll_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_lL_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_llV_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_lLV_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_x_l(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_x_p(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_x_L(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_x_v(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_x_V(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_x_c(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_x_s(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_x_lL(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_l_l(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_lL_v(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_s_l(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_sl_l(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_slL_l(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_slllL_l(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenFormat_slllLlL_l(long, long, PyObject *, long (*)(PyObject *));
/* Some custom token handling functions for when the special case is
 * too complex to be nicely handled with a helper function. */
PyObject *tokenCustom_rVAR_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenCustom_zVAR_x(long, long, PyObject *, long (*)(PyObject *));
PyObject *tokenCustom_zVAR_V(long, long, PyObject *, long (*)(PyObject *));

/* Helper functions. */
/* A helper function is sometimes passed to the third level token
 * handler function to assist in determining memory allocations.
 * The CDFlib function requires us to pre-allocate the memory for
 * returned data.  Typically the C programmer calling CDFlib would
 * have already performed introspection on the data to determine
 * the correct allocation size.  Since we're being called from
 * Python, we're going to attempt to be Pythonic and handle this
 * magically.
 *
 * In general, functions which return a list and functions which either
 * take or return a value or value list will require a helper function. */
long getSize(long);
long getToken(long);
long tokenHelper_firstToken_(PyObject *);
long typeHelper_rVAR_(PyObject *);
long typeHelper_zVAR_(PyObject *);
long typeHelper_rVARs_(PyObject *);
long typeHelper_zVARs_(PyObject *);
long hyperAllocHelper_rVAR_(PyObject *);
long hyperAllocHelper_zVAR_(PyObject *);
long helper_GET_rVARs_NUMDIMS_(PyObject *);
long helper_GET_zVAR_NUMDIMS_(PyObject *);
long helper_GET_gENTRY_DATATYPE_(PyObject *);
long helper_GET_rENTRY_DATATYPE_(PyObject *);
long helper_GET_zENTRY_DATATYPE_(PyObject *);

/* Second level token handling tables. */
static CdfSecondTierToken cdf_internal_token_tables_CLOSE_[] = {
    /* Valid, token, function, helper */
    {1, CDF_, &tokenFormat_x_x, 0, NULL},
    {1, rVAR_, &tokenFormat_x_x, 0, NULL},
    {1, zVAR_, &tokenFormat_x_x, 0, NULL},
    SECOND_TIER_TOKEN_TABLE_TERMINATOR()
};

static CdfSecondTierToken cdf_internal_token_tables_CONFIRM_[] = {
    /* Valid, token, function, helper */
    {1, ATTR_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_CACHESIZE_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_DECODING_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_NEGtoPOSfp0_MODE_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_READONLY_MODE_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_zMODE_, &tokenFormat_x_l, 0, NULL},
    {1, gENTRY_, &tokenFormat_x_l, 0, NULL},
    {1, rENTRY_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_CACHESIZE_, &tokenFormat_x_l, 0, NULL},
    {1, rVARs_RECCOUNT_, &tokenFormat_x_l, 0, NULL},
    {1, rVARs_RECINTERVAL_, &tokenFormat_x_l, 0, NULL},
    {1, rVARs_RECNUMBER_, &tokenFormat_x_l, 0, NULL},
    {1, zENTRY_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_CACHESIZE_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_RECCOUNT_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_RECINTERVAL_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_RECNUMBER_, &tokenFormat_x_l, 0, NULL},
    {1, ATTR_EXISTENCE_, &tokenFormat_s_x, 1, NULL},
    {1, rVAR_EXISTENCE_, &tokenFormat_s_x, 1, NULL},
    {1, zVAR_EXISTENCE_, &tokenFormat_s_x, 1, NULL},
    {1, CDF_NAME_, &tokenFormat_x_s, 0, NULL},
    {1, CURgENTRY_EXISTENCE_, &tokenFormat_x_x, 0, NULL},
    {1, CURrENTRY_EXISTENCE_, &tokenFormat_x_x, 0, NULL},
    {1, CURzENTRY_EXISTENCE_, &tokenFormat_x_x, 0, NULL},
    {1, gENTRY_EXISTENCE_, &tokenFormat_l_x, 1, NULL},
    {1, rENTRY_EXISTENCE_, &tokenFormat_l_x, 1, NULL},
    {1, zENTRY_EXISTENCE_, &tokenFormat_l_x, 1, NULL},
    {1, rVAR_SEQPOS_, &tokenFormat_x_lL, 0, NULL},
    {1, zVAR_SEQPOS_, &tokenFormat_x_lL, 0, NULL},
    {1, rVARs_DIMCOUNTS_, &tokenFormat_x_L, 0, NULL},
    {1, rVARs_DIMINDICES_, &tokenFormat_x_L, 0, NULL},
    {1, rVARs_DIMINTERVALS_, &tokenFormat_x_L, 0, NULL},
    {1, zVAR_DIMCOUNTS_, &tokenFormat_x_L, 0, NULL},
    {1, zVAR_DIMINDICES_, &tokenFormat_x_L, 0, NULL},
    {1, zVAR_DIMINTERVALS_, &tokenFormat_x_L, 0, NULL},
    SECOND_TIER_TOKEN_TABLE_TERMINATOR()
};

static CdfSecondTierToken cdf_internal_token_tables_CREATE_[] = {
    /* Valid, token, function, helper */
    {1, ATTR_, &tokenFormat_sl_l, 2, NULL},
    {1, CDF_, &tokenFormat_slL_l, 3, NULL},
    {1, rVAR_, &tokenFormat_slllL_l, 5, NULL},
    {1, zVAR_, &tokenFormat_slllLlL_l, 7, NULL},
    SECOND_TIER_TOKEN_TABLE_TERMINATOR()
};

static CdfSecondTierToken cdf_internal_token_tables_DELETE_[] = {
    /* Valid, token, function, helper */
    {1, ATTR_, &tokenFormat_x_x, 0, NULL},
    {1, CDF_, &tokenFormat_x_x, 0, NULL},
    {1, gENTRY_, &tokenFormat_x_x, 0, NULL},
    {1, rENTRY_, &tokenFormat_x_x, 0, NULL},
    {1, rVAR_, &tokenFormat_x_x, 0, NULL},
    {1, zENTRY_, &tokenFormat_x_x, 0, NULL},
    {1, zVAR_, &tokenFormat_x_x, 0, NULL},
    SECOND_TIER_TOKEN_TABLE_TERMINATOR()
};

static CdfSecondTierToken cdf_internal_token_tables_GET_[] = {
    /* Valid, token, function, helper */
    {1, ATTR_MAXgENTRY_, &tokenFormat_x_l, 0, NULL},
    {1, ATTR_MAXrENTRY_, &tokenFormat_x_l, 0, NULL},
    {1, ATTR_MAXzENTRY_, &tokenFormat_x_l, 0, NULL},
    {1, ATTR_NUMgENTRIES_, &tokenFormat_x_l, 0, NULL},
    {1, ATTR_NUMrENTRIES_, &tokenFormat_x_l, 0, NULL},
    {1, ATTR_NUMzENTRIES_, &tokenFormat_x_l, 0, NULL},
    {1, ATTR_SCOPE_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_ENCODING_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_FORMAT_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_INCREMENT_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_MAJORITY_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_NUMATTRS_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_NUMgATTRS_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_NUMrVARS_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_NUMvATTRS_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_NUMzVARS_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_RELEASE_, &tokenFormat_x_l, 0, NULL},
    {1, CDF_VERSION_, &tokenFormat_x_l, 0, NULL},
    {1, gENTRY_DATATYPE_, &tokenFormat_x_l, 0, NULL},
    {1, gENTRY_NUMELEMS_, &tokenFormat_x_l, 0, NULL},
    {1, LIB_INCREMENT_, &tokenFormat_x_l, 0, NULL},
    {1, LIB_RELEASE_, &tokenFormat_x_l, 0, NULL},
    {1, LIB_VERSION_, &tokenFormat_x_l, 0, NULL},
    {1, rENTRY_DATATYPE_, &tokenFormat_x_l, 0, NULL},
    {1, rENTRY_NUMELEMS_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_DATATYPE_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_EXTENDRECS_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_MAXallocREC_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_MAXREC_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_nINDEXENTRIES_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_nINDEXRECORDS_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_NUMELEMS_, &tokenFormat_x_l, 0, NULL},
    {1, rVAR_RECVARY_, &tokenFormat_x_l, 0, NULL},
    {1, rVARs_MAXREC_, &tokenFormat_x_l, 0, NULL},
    {1, rVARs_NUMDIMS_, &tokenFormat_x_l, 0, NULL},
    {1, zENTRY_DATATYPE_, &tokenFormat_x_l, 0, NULL},
    {1, zENTRY_NUMELEMS_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_DATATYPE_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_EXTENDRECS_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_MAXallocREC_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_MAXREC_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_nINDEXENTRIES_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_nINDEXRECORDS_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_NUMDIMS_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_NUMELEMS_, &tokenFormat_x_l, 0, NULL},
    {1, zVAR_RECVARY_, &tokenFormat_x_l, 0, NULL},
    {1, zVARs_MAXREC_, &tokenFormat_x_l, 0, NULL},
    {1, ATTR_NAME_, &tokenFormat_x_s, 0, NULL},
    {1, ATTR_NUMBER_, &tokenFormat_s_l, 1, NULL},
    {1, rVAR_NUMBER_, &tokenFormat_s_l, 1, NULL},
    {1, zVAR_NUMBER_, &tokenFormat_s_l, 1, NULL},
    {1, CDF_COPYRIGHT_, &tokenFormat_x_s, 0, NULL},
    {1, LIB_COPYRIGHT_, &tokenFormat_x_s, 0, NULL},
    {1, DATATYPE_SIZE_, &tokenFormat_l_l, 1, NULL},
    {1, gENTRY_DATA_, &tokenFormat_x_V, 0, &helper_GET_gENTRY_DATATYPE_},
    {1, rENTRY_DATA_, &tokenFormat_x_V, 0, &helper_GET_rENTRY_DATATYPE_},
    {1, rVAR_DATA_, &tokenFormat_x_v, 0,& typeHelper_rVAR_},
    {1, rVAR_HYPERDATA_, &tokenFormat_x_p, 0, &hyperAllocHelper_rVAR_},
    {1, rVAR_PADVALUE_, &tokenFormat_x_v, 0, &typeHelper_rVAR_},
    {1, rVAR_SEQDATA_, &tokenFormat_x_v, 0, &typeHelper_rVAR_},
    {1, zENTRY_DATA_, &tokenFormat_x_V, 0, &helper_GET_zENTRY_DATATYPE_},
    {1, zVAR_DATA_, &tokenFormat_x_v, 0, &typeHelper_zVAR_},
    {1, zVAR_HYPERDATA_, &tokenCustom_zVAR_V, 0, &hyperAllocHelper_zVAR_},
    {1, zVAR_PADVALUE_, &tokenFormat_x_v, 0, &typeHelper_zVAR_},
    {1, zVAR_SEQDATA_, &tokenFormat_x_v, 0, &typeHelper_zVAR_},
    {1, LIB_subINCREMENT_, &tokenFormat_x_c, 0, NULL},
    {1, rVAR_DIMVARYS_, &tokenFormat_x_L, 0, &helper_GET_rVARs_NUMDIMS_},
    {1, rVARs_DIMSIZES_, &tokenFormat_x_L, 0, &helper_GET_rVARs_NUMDIMS_},
    {1, zVAR_DIMSIZES_, &tokenFormat_x_L, 0, &helper_GET_zVAR_NUMDIMS_},
    {1, zVAR_DIMVARYS_, &tokenFormat_x_L, 0, &helper_GET_zVAR_NUMDIMS_},
    {1, rVAR_NAME_, &tokenFormat_x_s, 0, NULL},
    {1, zVAR_NAME_, &tokenFormat_x_s, 0, NULL},
    {1, rVARs_RECDATA_, &tokenFormat_lL_v, 0, NULL},
    {1, zVARs_RECDATA_, &tokenFormat_lL_v, 0, NULL},
    {1, STATUS_TEXT_, &tokenFormat_x_s, 0, NULL},
    SECOND_TIER_TOKEN_TABLE_TERMINATOR()
};

static CdfSecondTierToken cdf_internal_token_tables_OPEN_[] = {
    /* Valid, token, function, helper */
    {1, CDF_, &tokenFormat_s_l, 1, NULL},
    SECOND_TIER_TOKEN_TABLE_TERMINATOR()
};

static CdfSecondTierToken cdf_internal_token_tables_PUT_[] = {
    /* Valid, token, function, helper */
    {1, ATTR_NAME_, &tokenFormat_s_x, 1, NULL},
    {1, rVAR_NAME_, &tokenFormat_s_x, 1, NULL},
    {1, zVAR_NAME_, &tokenFormat_s_x, 1, NULL},
    {1, ATTR_SCOPE_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_ENCODING_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_FORMAT_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_MAJORITY_, &tokenFormat_l_x, 1, NULL},
    {1, rVAR_ALLOCATERECS_, &tokenFormat_l_x, 1, NULL},
    {1, rVAR_EXTENDRECS_, &tokenFormat_l_x, 1, NULL},
    {1, rVAR_INITIALRECS_, &tokenFormat_l_x, 1, NULL},
    {1, rVAR_RECVARY_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_ALLOCATERECS_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_EXTENDRECS_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_INITIALRECS_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_RECVARY_, &tokenFormat_l_x, 1, NULL},
    {1, rVAR_DATA_, &tokenCustom_rVAR_x, 1, NULL},
    {1, rVAR_HYPERDATA_, &tokenFormat_p_x, 1, &typeHelper_rVAR_},
    {1, rVAR_PADVALUE_, &tokenFormat_p_x, 1, NULL},
    {1, rVAR_SEQDATA_, &tokenFormat_p_x, 1, NULL},
    {1, zVAR_DATA_, &tokenCustom_zVAR_x, 1, NULL},
    {1, zVAR_HYPERDATA_, &tokenFormat_p_x, 1, &typeHelper_zVAR_},
    {1, zVAR_PADVALUE_, &tokenFormat_p_x, 1, NULL},
    {1, zVAR_SEQDATA_, &tokenFormat_p_x, 1, NULL},
    {1, gENTRY_DATA_, &tokenFormat_llV_x, 3, &tokenHelper_firstToken_},
    {1, rENTRY_DATA_, &tokenFormat_llV_x, 3, &tokenHelper_firstToken_},
    {1, zENTRY_DATA_, &tokenFormat_llV_x, 3, &tokenHelper_firstToken_},
    {1, gENTRY_DATASPEC_, &tokenFormat_ll_x, 2, NULL},
    {1, rENTRY_DATASPEC_, &tokenFormat_ll_x, 2, NULL},
    {1, rVAR_DATASPEC_, &tokenFormat_ll_x, 2, NULL},
    {1, zENTRY_DATASPEC_, &tokenFormat_ll_x, 2, NULL},
    {1, zVAR_DATASPEC_, &tokenFormat_ll_x, 2, NULL},
    {1, rVAR_DIMVARYS_, &tokenFormat_L_x, 1, NULL},
    {1, zVAR_DIMVARYS_, &tokenFormat_L_x, 1, NULL},
    {1, rVARs_RECDATA_, &tokenFormat_lLV_x, 3, &typeHelper_rVARs_},
    {1, zVARs_RECDATA_, &tokenFormat_lLV_x, 3, &typeHelper_zVARs_},
    SECOND_TIER_TOKEN_TABLE_TERMINATOR()
};

static CdfSecondTierToken cdf_internal_token_tables_SELECT_[] = {
    /* Valid, token, function, helper */
    {1, ATTR_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_CACHESIZE_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_DECODING_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_NEGtoPOSfp0_MODE_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_READONLY_MODE_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_zMODE_, &tokenFormat_l_x, 1, NULL},
    {1, gENTRY_, &tokenFormat_l_x, 1, NULL},
    {1, rENTRY_, &tokenFormat_l_x, 1, NULL},
    {1, rVAR_, &tokenFormat_l_x, 1, NULL},
    {1, rVAR_CACHESIZE_, &tokenFormat_l_x, 1, NULL},
    {1, rVARs_CACHESIZE_, &tokenFormat_l_x, 1, NULL},
    {1, rVARs_RECCOUNT_, &tokenFormat_l_x, 1, NULL},
    {1, rVARs_RECINTERVAL_, &tokenFormat_l_x, 1, NULL},
    {1, rVARs_RECNUMBER_, &tokenFormat_l_x, 1, NULL},
    {1, zENTRY_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_CACHESIZE_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_RECCOUNT_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_RECINTERVAL_, &tokenFormat_l_x, 1, NULL},
    {1, zVAR_RECNUMBER_, &tokenFormat_l_x, 1, NULL},
    {1, zVARs_CACHESIZE_, &tokenFormat_l_x, 1, NULL},
    {1, zVARs_RECNUMBER_, &tokenFormat_l_x, 1, NULL},
    {1, ATTR_NAME_, &tokenFormat_s_x, 1, NULL},
    {1, CDF_, &tokenFormat_l_x, 1, NULL},
    {1, CDF_STATUS_, &tokenFormat_l_x, 1, NULL},
    {1, rENTRY_NAME_, &tokenFormat_s_x, 1, NULL},
    {1, rVAR_NAME_, &tokenFormat_s_x, 1, NULL},
    {1, zENTRY_NAME_, &tokenFormat_s_x, 1, NULL},
    {1, zVAR_NAME_, &tokenFormat_s_x, 1, NULL},
    {1, rVAR_SEQPOS_, &tokenFormat_lL_x, 2, NULL},
    {1, zVAR_SEQPOS_, &tokenFormat_lL_x, 2, NULL},
    {1, rVARs_DIMCOUNTS_, &tokenFormat_L_x, 1, NULL},
    {1, rVARs_DIMINDICES_, &tokenFormat_L_x, 1, NULL},
    {1, rVARs_DIMINTERVALS_, &tokenFormat_L_x, 1, NULL},
    {1, zVAR_DIMCOUNTS_, &tokenFormat_L_x, 1, NULL},
    {1, zVAR_DIMINDICES_, &tokenFormat_L_x, 1, NULL},
    {1, zVAR_DIMINTERVALS_, &tokenFormat_L_x, 1, NULL},
    SECOND_TIER_TOKEN_TABLE_TERMINATOR()
};

/* First level token handling functions. */
DECLARE_FIRST_TIER_TOKEN(CLOSE_)
DECLARE_FIRST_TIER_TOKEN(CONFIRM_)
DECLARE_FIRST_TIER_TOKEN(CREATE_)
DECLARE_FIRST_TIER_TOKEN(DELETE_)
DECLARE_FIRST_TIER_TOKEN(GET_)
DECLARE_FIRST_TIER_TOKEN(OPEN_)
DECLARE_FIRST_TIER_TOKEN(PUT_)
DECLARE_FIRST_TIER_TOKEN(SELECT_)

/* First level token tables. */
static CdfFirstTierToken CdfAPITokens[] = {
    /* Valid, token, function, helper */
    FIRST_TIER_TOKEN(CLOSE_)
    FIRST_TIER_TOKEN(CONFIRM_)
    FIRST_TIER_TOKEN(CREATE_)
    FIRST_TIER_TOKEN(DELETE_)
    FIRST_TIER_TOKEN(GET_)
    FIRST_TIER_TOKEN(OPEN_)
    FIRST_TIER_TOKEN(PUT_)
    FIRST_TIER_TOKEN(SELECT_)
    FIRST_TIER_TOKEN_TABLE_TERMINATOR()
};

#endif
