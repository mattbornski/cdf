/**
 * Include requisisite headers for this module.
 **/
/* My own header. */
#include "internal.h"



/**
 * Declare common objects for this module.  Do this before defining the
 * methods, as the methods may want to reference them.
 **/
/* An error. */
static PyObject *CdfInternalError;



/**
 * The core Python extension code.
 */
/* Implements the CDF "Internal interface". */
static PyObject *cdf_internal_CDFlib(PyObject *self, PyObject *args) {
    if (PySequence_Check(args)) {
        PyObject *ret = NULL;
        PyObject *type = NULL, *value = NULL, *traceback = NULL;
        /* Clean slate before processing. */
        PyErr_Fetch(&type, &value, &traceback);
        /* Process the sequence of tokens through the token->function
         * tables I've created for the internal interface. */
        ret = CdfFirstTierTokenHandler(args, CdfAPITokens);
        if (!PyErr_Occurred()) {
            PyErr_Restore(type, value, traceback);
            return ret;
        } else {
            Py_XDECREF(type);
            Py_XDECREF(value);
            Py_XDECREF(traceback);
            Py_XDECREF(ret);
            return NULL;
        }
    } else {
        return NULL;
    }
}



/**
 * Define method table for this module.  Do this after declaring
 * or defining methods but before defining initialization, as
 * the initialization must register this table.
 **/
static PyMethodDef CdfInternalMethods[] = {
    {"CDFlib", cdf_internal_CDFlib, METH_VARARGS, "CDF internal API"},
    {NULL, NULL, 0, NULL}
};

/* Define initialization for this module. */
PyMODINIT_FUNC
initinternal(void)
{
    PyObject *module;

    module = Py_InitModule("cdf.internal", CdfInternalMethods);
    if (module == NULL) {
        return;
    }

    /* Define common objects which were previously declared. */
    /* An error. */
    CdfInternalError = PyErr_NewException("cdf.internal.error", NULL, NULL);
    Py_INCREF(CdfInternalError);
    PyModule_AddObject(module, "error", CdfInternalError);

    /* Errors. */
    DEFINE_AND_EXPORT_ERROR(ATTR_NAME_TRUNC)
    DEFINE_AND_EXPORT_ERROR(CDF_NAME_TRUNC)
    DEFINE_AND_EXPORT_ERROR(VAR_NAME_TRUNC)
    DEFINE_AND_EXPORT_ERROR(NEGATIVE_FP_ZERO)
    DEFINE_AND_EXPORT_ERROR(FUTURE_CDF)
    DEFINE_AND_EXPORT_ERROR(ATTR_EXISTS)
    DEFINE_AND_EXPORT_ERROR(BAD_CDF_ID)
    DEFINE_AND_EXPORT_ERROR(BAD_DATA_TYPE)
    DEFINE_AND_EXPORT_ERROR(BAD_DIM_SIZE)
    DEFINE_AND_EXPORT_ERROR(BAD_DIM_INDEX)
    DEFINE_AND_EXPORT_ERROR(BAD_ENCODING)
    DEFINE_AND_EXPORT_ERROR(BAD_MAJORITY)
    DEFINE_AND_EXPORT_ERROR(BAD_NUM_DIMS)
    DEFINE_AND_EXPORT_ERROR(BAD_REC_NUM)
    DEFINE_AND_EXPORT_ERROR(BAD_SCOPE)
    DEFINE_AND_EXPORT_ERROR(BAD_NUM_ELEMS)
    DEFINE_AND_EXPORT_ERROR(CDF_OPEN_ERROR)
    DEFINE_AND_EXPORT_ERROR(CDF_EXISTS)
    DEFINE_AND_EXPORT_ERROR(BAD_FORMAT)
    DEFINE_AND_EXPORT_ERROR(BAD_ALLOCATE_RECS)
    DEFINE_AND_EXPORT_ERROR(BAD_CDF_EXTENSION)
    DEFINE_AND_EXPORT_ERROR(NO_SUCH_ATTR)
    DEFINE_AND_EXPORT_ERROR(NO_SUCH_ENTRY)
    DEFINE_AND_EXPORT_ERROR(NO_SUCH_VAR)
    DEFINE_AND_EXPORT_ERROR(VAR_READ_ERROR)
    DEFINE_AND_EXPORT_ERROR(VAR_WRITE_ERROR)
    DEFINE_AND_EXPORT_ERROR(BAD_ARGUMENT)
    DEFINE_AND_EXPORT_ERROR(IBM_PC_OVERFLOW)
    DEFINE_AND_EXPORT_ERROR(TOO_MANY_VARS)
    DEFINE_AND_EXPORT_ERROR(VAR_EXISTS)
    DEFINE_AND_EXPORT_ERROR(BAD_MALLOC)
    DEFINE_AND_EXPORT_ERROR(NOT_A_CDF)
    DEFINE_AND_EXPORT_ERROR(CORRUPTED_V2_CDF)
    DEFINE_AND_EXPORT_ERROR(VAR_OPEN_ERROR)
    DEFINE_AND_EXPORT_ERROR(BAD_INITIAL_RECS)
    DEFINE_AND_EXPORT_ERROR(BAD_EXTEND_RECS)
    DEFINE_AND_EXPORT_ERROR(END_OF_VAR)
    DEFINE_AND_EXPORT_ERROR(BAD_CDFSTATUS)
    DEFINE_AND_EXPORT_ERROR(CDF_INTERNAL_ERROR)
    DEFINE_AND_EXPORT_ERROR(BAD_NUM_VARS)
    DEFINE_AND_EXPORT_ERROR(BAD_REC_COUNT)
    DEFINE_AND_EXPORT_ERROR(BAD_REC_INTERVAL)
    DEFINE_AND_EXPORT_ERROR(BAD_DIM_COUNT)
    DEFINE_AND_EXPORT_ERROR(BAD_DIM_INTERVAL)
    DEFINE_AND_EXPORT_ERROR(BAD_VAR_NUM)
    DEFINE_AND_EXPORT_ERROR(BAD_ATTR_NUM)
    DEFINE_AND_EXPORT_ERROR(BAD_ENTRY_NUM)
    DEFINE_AND_EXPORT_ERROR(BAD_ATTR_NAME)
    DEFINE_AND_EXPORT_ERROR(BAD_VAR_NAME)
    DEFINE_AND_EXPORT_ERROR(NO_ATTR_SELECTED)
    DEFINE_AND_EXPORT_ERROR(NO_ENTRY_SELECTED)
    DEFINE_AND_EXPORT_ERROR(NO_VAR_SELECTED)
    DEFINE_AND_EXPORT_ERROR(BAD_CDF_NAME)
    DEFINE_AND_EXPORT_ERROR(CANNOT_CHANGE)
    DEFINE_AND_EXPORT_ERROR(NO_STATUS_SELECTED)
    DEFINE_AND_EXPORT_ERROR(NO_CDF_SELECTED)
    DEFINE_AND_EXPORT_ERROR(READ_ONLY_DISTRIBUTION)
    DEFINE_AND_EXPORT_ERROR(CDF_CLOSE_ERROR)
    DEFINE_AND_EXPORT_ERROR(VAR_CLOSE_ERROR)
    DEFINE_AND_EXPORT_ERROR(BAD_FNC_OR_ITEM)
    DEFINE_AND_EXPORT_ERROR(ILLEGAL_ON_V1_CDF)
    DEFINE_AND_EXPORT_ERROR(CDH_OPEN_ERROR)
    DEFINE_AND_EXPORT_ERROR(CDH_CLOSE_ERROR)
    DEFINE_AND_EXPORT_ERROR(BAD_CACHE_SIZE)
    DEFINE_AND_EXPORT_ERROR(CDF_CREATE_ERROR)
    DEFINE_AND_EXPORT_ERROR(NO_SUCH_CDF)
    DEFINE_AND_EXPORT_ERROR(VAR_CREATE_ERROR)
    DEFINE_AND_EXPORT_ERROR(READ_ONLY_MODE)
    DEFINE_AND_EXPORT_ERROR(ILLEGAL_IN_zMODE)
    DEFINE_AND_EXPORT_ERROR(BAD_zMODE)
    DEFINE_AND_EXPORT_ERROR(BAD_READONLY_MODE)
    DEFINE_AND_EXPORT_ERROR(CDF_READ_ERROR)
    DEFINE_AND_EXPORT_ERROR(CDF_WRITE_ERROR)
    DEFINE_AND_EXPORT_ERROR(ILLEGAL_FOR_SCOPE)
    DEFINE_AND_EXPORT_ERROR(NO_MORE_ACCESS)
    DEFINE_AND_EXPORT_ERROR(BAD_DECODING)
    DEFINE_AND_EXPORT_ERROR(BAD_NEGtoPOSfp0_MODE)
    DEFINE_AND_EXPORT_ERROR(UNSUPPORTED_OPERATION)
    DEFINE_AND_EXPORT_ERROR(NO_WRITE_ACCESS)
    DEFINE_AND_EXPORT_ERROR(NO_DELETE_ACCESS)

    /* Constants. */
    DEFINE_AND_EXPORT_CONSTANT(SINGLE_FILE)
    DEFINE_AND_EXPORT_CONSTANT(MULTI_FILE)

    DEFINE_AND_EXPORT_CONSTANT(CDF_BYTE)
    DEFINE_AND_EXPORT_CONSTANT(CDF_CHAR)
    DEFINE_AND_EXPORT_CONSTANT(CDF_INT1)
    DEFINE_AND_EXPORT_CONSTANT(CDF_UCHAR)
    DEFINE_AND_EXPORT_CONSTANT(CDF_UINT1)
    DEFINE_AND_EXPORT_CONSTANT(CDF_INT2)
    DEFINE_AND_EXPORT_CONSTANT(CDF_UINT2)
    DEFINE_AND_EXPORT_CONSTANT(CDF_INT4)
    DEFINE_AND_EXPORT_CONSTANT(CDF_UINT4)
    DEFINE_AND_EXPORT_CONSTANT(CDF_REAL4)
    DEFINE_AND_EXPORT_CONSTANT(CDF_FLOAT)
    DEFINE_AND_EXPORT_CONSTANT(CDF_REAL8)
    DEFINE_AND_EXPORT_CONSTANT(CDF_DOUBLE)
    DEFINE_AND_EXPORT_CONSTANT(CDF_EPOCH)
    DEFINE_AND_EXPORT_CONSTANT(CDF_EPOCH16)

    DEFINE_AND_EXPORT_CONSTANT(HOST_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(NETWORK_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(VAX_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(ALPHAVMSd_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(ALPHAVMSg_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(SUN_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(SGi_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(DECSTATION_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(ALPHAOSF1_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(IBMRS_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(HP_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(IBMPC_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(NeXT_ENCODING)
    DEFINE_AND_EXPORT_CONSTANT(MAC_ENCODING)

    DEFINE_AND_EXPORT_CONSTANT(HOST_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(NETWORK_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(VAX_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(ALPHAVMSd_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(ALPHAVMSg_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(SUN_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(SGi_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(DECSTATION_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(ALPHAOSF1_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(IBMRS_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(HP_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(IBMPC_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(NeXT_DECODING)
    DEFINE_AND_EXPORT_CONSTANT(MAC_DECODING)

    DEFINE_AND_EXPORT_CONSTANT(ROW_MAJOR)
    DEFINE_AND_EXPORT_CONSTANT(COL_MAJOR)

    DEFINE_AND_EXPORT_CONSTANT(VARY)
    DEFINE_AND_EXPORT_CONSTANT(NOVARY)

    DEFINE_AND_EXPORT_CONSTANT(GLOBAL_SCOPE)
    DEFINE_AND_EXPORT_CONSTANT(VARIABLE_SCOPE)

    DEFINE_AND_EXPORT_CONSTANT(READONLYon)
    DEFINE_AND_EXPORT_CONSTANT(READONLYoff)

    DEFINE_AND_EXPORT_CONSTANT(zMODEoff)
    DEFINE_AND_EXPORT_CONSTANT(zMODEon1)
    DEFINE_AND_EXPORT_CONSTANT(zMODEon2)

    DEFINE_AND_EXPORT_CONSTANT(NEGtoPOSfp0on)
    DEFINE_AND_EXPORT_CONSTANT(NEGtoPOSfp0off)

    DEFINE_AND_EXPORT_CONSTANT(CDF_MAX_DIMS)

    DEFINE_AND_EXPORT_CONSTANT(CDF_PATHNAME_LEN)
    DEFINE_AND_EXPORT_CONSTANT(CDF_VAR_NAME_LEN)
    DEFINE_AND_EXPORT_CONSTANT(CDF_ATTR_NAME_LEN)
    DEFINE_AND_EXPORT_CONSTANT(CDF_COPYRIGHT_LEN)
    DEFINE_AND_EXPORT_CONSTANT(CDF_STATUSTEXT_LEN)

    /* Tokens. */
    DEFINE_AND_EXPORT_CONSTANT(CREATE_)
    DEFINE_AND_EXPORT_CONSTANT(OPEN_)
    DEFINE_AND_EXPORT_CONSTANT(DELETE_)
    DEFINE_AND_EXPORT_CONSTANT(CLOSE_)
    DEFINE_AND_EXPORT_CONSTANT(SELECT_)
    DEFINE_AND_EXPORT_CONSTANT(CONFIRM_)
    DEFINE_AND_EXPORT_CONSTANT(GET_)
    DEFINE_AND_EXPORT_CONSTANT(PUT_)

    DEFINE_AND_EXPORT_CONSTANT(NULL_)

    DEFINE_AND_EXPORT_CONSTANT(CDF_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_NAME_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_ENCODING_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_DECODING_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_MAJORITY_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_FORMAT_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_COPYRIGHT_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_NUMrVARS_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_NUMzVARS_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_NUMATTRS_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_NUMgATTRS_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_NUMvATTRS_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_VERSION_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_RELEASE_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_INCREMENT_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_STATUS_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_READONLY_MODE_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_zMODE_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_NEGtoPOSfp0_MODE_)
    DEFINE_AND_EXPORT_CONSTANT(LIB_COPYRIGHT_)
    DEFINE_AND_EXPORT_CONSTANT(LIB_VERSION_)
    DEFINE_AND_EXPORT_CONSTANT(LIB_RELEASE_)
    DEFINE_AND_EXPORT_CONSTANT(LIB_INCREMENT_)
    DEFINE_AND_EXPORT_CONSTANT(LIB_subINCREMENT_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_NUMDIMS_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_DIMSIZES_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_MAXREC_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_RECDATA_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_RECNUMBER_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_RECCOUNT_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_RECINTERVAL_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_DIMINDICES_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_DIMCOUNTS_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_DIMINTERVALS_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_NAME_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_DATATYPE_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_NUMELEMS_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_RECVARY_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_DIMVARYS_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_NUMBER_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_DATA_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_HYPERDATA_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_SEQDATA_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_SEQPOS_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_MAXREC_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_MAXallocREC_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_DATASPEC_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_PADVALUE_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_INITIALRECS_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_EXTENDRECS_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_nINDEXRECORDS_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_nINDEXENTRIES_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_EXISTENCE_)
    DEFINE_AND_EXPORT_CONSTANT(zVARs_MAXREC_)
    DEFINE_AND_EXPORT_CONSTANT(zVARs_RECDATA_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_NAME_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_DATATYPE_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_NUMELEMS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_NUMDIMS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_DIMSIZES_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_RECVARY_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_DIMVARYS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_NUMBER_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_DATA_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_HYPERDATA_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_SEQDATA_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_SEQPOS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_MAXREC_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_MAXallocREC_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_DATASPEC_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_PADVALUE_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_INITIALRECS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_EXTENDRECS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_nINDEXRECORDS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_nINDEXENTRIES_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_EXISTENCE_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_RECNUMBER_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_RECCOUNT_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_RECINTERVAL_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_DIMINDICES_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_DIMCOUNTS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_DIMINTERVALS_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_SCOPE_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_NAME_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_NUMBER_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_MAXgENTRY_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_NUMgENTRIES_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_MAXrENTRY_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_NUMrENTRIES_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_MAXzENTRY_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_NUMzENTRIES_)
    DEFINE_AND_EXPORT_CONSTANT(ATTR_EXISTENCE_)
    DEFINE_AND_EXPORT_CONSTANT(gENTRY_)
    DEFINE_AND_EXPORT_CONSTANT(gENTRY_EXISTENCE_)
    DEFINE_AND_EXPORT_CONSTANT(gENTRY_DATATYPE_)
    DEFINE_AND_EXPORT_CONSTANT(gENTRY_NUMELEMS_)
    DEFINE_AND_EXPORT_CONSTANT(gENTRY_DATASPEC_)
    DEFINE_AND_EXPORT_CONSTANT(gENTRY_DATA_)
    DEFINE_AND_EXPORT_CONSTANT(rENTRY_)
    DEFINE_AND_EXPORT_CONSTANT(rENTRY_NAME_)
    DEFINE_AND_EXPORT_CONSTANT(rENTRY_EXISTENCE_)
    DEFINE_AND_EXPORT_CONSTANT(rENTRY_DATATYPE_)
    DEFINE_AND_EXPORT_CONSTANT(rENTRY_NUMELEMS_)
    DEFINE_AND_EXPORT_CONSTANT(rENTRY_DATASPEC_)
    DEFINE_AND_EXPORT_CONSTANT(rENTRY_DATA_)
    DEFINE_AND_EXPORT_CONSTANT(zENTRY_)
    DEFINE_AND_EXPORT_CONSTANT(zENTRY_NAME_)
    DEFINE_AND_EXPORT_CONSTANT(zENTRY_EXISTENCE_)
    DEFINE_AND_EXPORT_CONSTANT(zENTRY_DATATYPE_)
    DEFINE_AND_EXPORT_CONSTANT(zENTRY_NUMELEMS_)
    DEFINE_AND_EXPORT_CONSTANT(zENTRY_DATASPEC_)
    DEFINE_AND_EXPORT_CONSTANT(zENTRY_DATA_)
    DEFINE_AND_EXPORT_CONSTANT(STATUS_TEXT_)
    DEFINE_AND_EXPORT_CONSTANT(CDF_CACHESIZE_)
    DEFINE_AND_EXPORT_CONSTANT(rVARs_CACHESIZE_)
    DEFINE_AND_EXPORT_CONSTANT(zVARs_CACHESIZE_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_CACHESIZE_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_CACHESIZE_)
    DEFINE_AND_EXPORT_CONSTANT(zVARs_RECNUMBER_)
    DEFINE_AND_EXPORT_CONSTANT(rVAR_ALLOCATERECS_)
    DEFINE_AND_EXPORT_CONSTANT(zVAR_ALLOCATERECS_)
    DEFINE_AND_EXPORT_CONSTANT(DATATYPE_SIZE_)
    DEFINE_AND_EXPORT_CONSTANT(CURgENTRY_EXISTENCE_)
    DEFINE_AND_EXPORT_CONSTANT(CURrENTRY_EXISTENCE_)
    DEFINE_AND_EXPORT_CONSTANT(CURzENTRY_EXISTENCE_)

    DEFINE_AND_EXPORT_CONSTANT(CDFwithSTATS_)
}



/**
 * Define helper methods of this module.
 **/
/* Helper function to look up message text for a CDF error. */
PyObject *message(long status) {
    PyObject *text = NULL;
    PyObject *output = NULL;
    PyObject *input = PyTuple_New(6);
    PyTuple_SetItem(input, 0, PyLong_FromLong(SELECT_));
    PyTuple_SetItem(input, 1, PyLong_FromLong(CDF_STATUS_));
    PyTuple_SetItem(input, 2, PyLong_FromLong(status));
    PyTuple_SetItem(input, 3, PyLong_FromLong(GET_));
    PyTuple_SetItem(input, 4, PyLong_FromLong(STATUS_TEXT_));
    PyTuple_SetItem(input, 5, PyLong_FromLong(NULL_));
    /* Before calling to get the error message, clear the existing
     * error indicator. */
    output = cdf_internal_CDFlib(NULL, input);
    Py_DecRef(input);
    text = PyTuple_GetItem(output, 0);
    Py_IncRef(text);
    Py_DecRef(output);
    return text;
}

/* Helper function to handle logic of throwing exceptions when
 * something goes awry in calls to CDFlib. */
int check(CDFstatus status) {
    if (status < CDF_WARN) {
        PyObject *text = message(status);
        PyErr_SetString(CdfInternalError, PyString_AsString(text));
        Py_DecRef(text);
        return 0;
    } else if (status < CDF_OK) {
        PyObject *text = message(status);
        PyErr_WarnEx(PyExc_Warning, PyString_AsString(text), 0);
        Py_DecRef(text);
        return 1;
    } else if (status > CDF_OK) {
        PyObject *text = message(status);
        PyErr_WarnEx(PyExc_Warning, PyString_AsString(text), 0);
        Py_DecRef(text);
        return 1;
    } else {
        return 1;
    }
}

/* Helper function to handle logic of throwing exceptions when
 * allocations fail. */
void *alloc(void *region) {
    if (region == NULL) {
        PyErr_NoMemory();
    }
    return region;
}

void *rebinFromPythonToC(PyObject *in, long type) {
    if (in != NULL) {
        Py_IncRef(in);
        void *out = NULL;
        if (PyString_Check(in) || PyUnicode_Check(in)) {
            long len;
            void *tmp = NULL;
            if (PyString_Check(in)) {
                /* Borrow the representation of the contents. */
                len = PyString_Size(in);
                tmp = (void *)PyString_AsString(in);
            } else {
                len = PyUnicode_GetSize(in);
                tmp = (void *)PyUnicode_AsASCIIString(in);
            }
            if (tmp != NULL) {
                out = alloc(calloc(sizeof(char), len + 1));
                if (out != NULL) {
                    memcpy(out, tmp, len + 1);
                }
            }
        } else {
            long len = PySequence_Check(in) ? PySequence_Size(in) : 1;
            long size = getSize(type);
            out = alloc(calloc(size, len));

            if (out != NULL) {
                long i;
                for (i = 0; i < len; i++) {
                    PyObject *item = in;
                    if (PySequence_Check(in)) {
                        item = PySequence_GetItem(in, i);
                    }

                    Py_IncRef(item);
                    PyObject *tuple = PyTuple_New(1);
                    PyTuple_SetItem(tuple, 0, item);
                    if ((type == CDF_REAL4)
                        || (type == CDF_FLOAT)
                        || (type == CDF_REAL8)
                        || (type == CDF_DOUBLE)
                        || (type == CDF_EPOCH)
                        || (type == CDF_EPOCH16)) {
                        if (size == sizeof(float)) {
                            float conv;
                            PyArg_ParseTuple(tuple, "f", &conv);
                            ((float *)(out))[i] = conv;
                        } else if (size == sizeof(double)) {
                            double conv;
                            PyArg_ParseTuple(tuple, "d", &conv);
                            ((double *)(out))[i] = conv;
                        } else {
                            Py_DecRef(tuple);
                            printf("cdf.internal: Unable to convert Python "
                                "floating point representation into C "
                                "floating point representation in %ld "
                                "bytes.\n", size);
                            free(out);
                            return NULL;
                        }
                    } else {
                        if (size == sizeof(char)) {
                            char conv;
                            PyArg_ParseTuple(tuple, "b", &conv);
                            ((char *)(out))[i] = conv;
                        } else if (size == sizeof(short)) {
                            short conv;
                            PyArg_ParseTuple(tuple, "h", &conv);
                            ((short *)(out))[i] = conv;
                        } else if (size == sizeof(int)) {
                            int conv;
                            PyArg_ParseTuple(tuple, "i", &conv);
                            ((int *)(out))[i] = conv;
                        } else if (size == sizeof(long)) {
                            long conv;
                            PyArg_ParseTuple(tuple, "l", &conv);
                            ((long *)(out))[i] = conv;
                        } else {
                            Py_DecRef(tuple);
                            printf("cdf.internal: Unable to convert Python "
                                "integer representation into C integer "
                                "representation in %ld bytes.\n", size);
                            free(out);
                            return NULL;
                        }
                    }
                    Py_DecRef(tuple);
                }
            }
        }
        Py_DecRef(in);
        return out;
    } else {
        printf("cdf.internal: Unable to convert Python list to C list "
            "because of NULL pointers.\n");
    }
    return NULL;
}

PyObject *castFromCdfToPython(long cdf_type, void *in) {
    PyObject *out = NULL;
    if (in != NULL) {
        switch (cdf_type) {
            /* Separate integral types from floating point types. */
            /* Furthermore separate by the length of each type. */
            case CDF_BYTE:
            case CDF_UINT1:
                out = Py_BuildValue("B", ((char *)in)[0]);
                break;
            case CDF_INT1:
                /* break */
            case CDF_INT2:
            case CDF_UINT2:
                /* break */
            case CDF_INT4:
            case CDF_UINT4:
                if (sizeof(long) == 4) {
                    long value = ((long *)in)[0];
                    out = PyLong_FromLong(value);
                } else if (sizeof(int) == 4) {
                    int value = ((int *)in)[0];
                    out = PyLong_FromLong((long)value);
                }
                break;
            case CDF_REAL4:
            case CDF_FLOAT:
                if (sizeof(float) == 4) {
                    double value = ((float *)in)[0];
                    out = PyFloat_FromDouble(value);
                } else {
                    double value = ((double *)in)[0];
                    out = PyFloat_FromDouble(value);
                }
                break;
            case CDF_REAL8:
            case CDF_DOUBLE:
            case CDF_EPOCH:
                if (sizeof(float) == 8) {
                    double value = ((float *)in)[0];
                    out = PyFloat_FromDouble(value);
                } else {
                    double value = ((double *)in)[0];
                    out = PyFloat_FromDouble(value);
                }
                break;
            case CDF_EPOCH16:
                {
                    double value = ((double *)in)[0];
                    out = PyFloat_FromDouble(value);
                    break;
                }
            case CDF_CHAR:
                {
                    long len = strlen((char *)in);
                    char *tmp = alloc(calloc(sizeof(char), len + 1));
                    memcpy(tmp, in, len + 1);
                    out = PyString_FromString(tmp);
                }
                break;
            default:
                printf("cdf.internal: CDF data type %ld of unknown size.\n",
                    cdf_type);
                break;
        }
    }
    return out;
}

/* Perform a GET_ query with the given token and return a long result. */
long getToken(long token) {
    PyObject *input = PyTuple_New(3);
    PyTuple_SetItem(input, 0, PyLong_FromLong(GET_));
    PyTuple_SetItem(input, 1, PyLong_FromLong(token));
    PyTuple_SetItem(input, 2, PyLong_FromLong(NULL_));
    PyObject *output = cdf_internal_CDFlib(NULL, input);
    Py_DecRef(input);
    long result = -1;
    if (output != NULL) {
        if (PyTuple_Check(output)) {
            result = PyLong_AsLong(PyTuple_GetItem(output, 0));
        }
        Py_DecRef(output);
    }
    return result;
}

/* Perform a GET_ query for the size of the given datatype token and return
 * a long result. */
long getSize(long type) {
    PyObject *input = PyTuple_New(4);
    PyTuple_SetItem(input, 0, PyLong_FromLong(GET_));
    PyTuple_SetItem(input, 1, PyLong_FromLong(DATATYPE_SIZE_));
    PyTuple_SetItem(input, 2, PyLong_FromLong(type));
    PyTuple_SetItem(input, 3, PyLong_FromLong(NULL_));
    PyObject *output = cdf_internal_CDFlib(NULL, input);
    Py_DecRef(input);
    long result = -1;
    if (output != NULL) {
        if (PyTuple_Check(output)) {
            result = PyLong_AsLong(PyTuple_GetItem(output, 0));
        }
        Py_DecRef(output);
    }
    return result;
}



/**
 * Helper functions to calculate sizes and lengths.
 */
/**
 * My type convention is hacktastic.  I must communicate the type of
 * data, as well as how many of them there are.  Because the "helper"
 * system has evolved to return only one long, and I'm not masochistic
 * enough to tackle that right now, I'm going to encode two bits of
 * info in one long.  Fancy.
 *
 * In particular, I'm going to multiply the type by 200 and add the size.
 * I chose 200 because it's small enough to allow all CDF data type codes
 * to fit in a long when multiplied by it, but large enough to allow a fair
 * size.
 */
#define TYPE_MOD 200L
long typeConventionHelper(long type_token, long len_token) {
    long type = getToken(type_token);
    long size = getToken(len_token);
    if (type == CDF_CHAR) {
        return -1 * size;
    } else {
        return TYPE_MOD * type + size;
    }
}

/* Return the first token. */
long tokenHelper_firstToken_(PyObject *tokens) {
    return PyLong_AsLong(PyTuple_GetItem(tokens, 0));
}

/* Look up the size of the datatypes of given rVARs.
 * Requires that the second token in a token tuple be a
 * list of the rVAR numbers, and that the rVARs all have
 * the same datatype. */
long typeHelper_rVARs_(PyObject *tokens) {
    long ret = -1;
    if ((tokens != NULL) && (PyTuple_Size(tokens) > 1)) {
        PyObject *list = PyTuple_GetItem(tokens, 1);
        if ((list != NULL) && (PySequence_Check(list)
            && (PySequence_Size(list) > 0))) {
            PyObject *first = PySequence_GetItem(list, 0);
            if (PyLong_Check(first) || PyInt_Check(first)) {
                /* Save off the current variable number. */
                PyObject *tmp = NULL;
                PyObject *saveme = NULL;
                PyObject *input = PyTuple_New(3);
                PyTuple_SetItem(input, 0, PyLong_FromLong(CONFIRM_));
                PyTuple_SetItem(input, 1, PyLong_FromLong(rVAR_));
                PyTuple_SetItem(input, 2, PyLong_FromLong(NULL_));
                tmp = cdf_internal_CDFlib(NULL, input);
                Py_XDECREF(input);
                saveme = PyTuple_GetItem(tmp, 0);
                Py_IncRef(saveme);
                Py_XDECREF(tmp);
                /* Inquire as to the size of the datatype for the first
                 * variable number in the list. */
                Py_IncRef(first);
                input = PyTuple_New(4);
                PyTuple_SetItem(input, 0, PyLong_FromLong(SELECT_));
                PyTuple_SetItem(input, 1, PyLong_FromLong(rVAR_));
                PyTuple_SetItem(input, 2, first);
                PyTuple_SetItem(input, 3, PyLong_FromLong(NULL_));
                tmp = cdf_internal_CDFlib(NULL, input);
                Py_XDECREF(tmp);
                ret = getSize(getToken(rVAR_DATATYPE_));
                /* Restore the previous variable number. */
                input = PyTuple_New(4);
                PyTuple_SetItem(input, 0, PyLong_FromLong(SELECT_));
                PyTuple_SetItem(input, 1, PyLong_FromLong(rVAR_));
                PyTuple_SetItem(input, 2, saveme);
                PyTuple_SetItem(input, 3, PyLong_FromLong(NULL_));
                tmp = cdf_internal_CDFlib(NULL, input);
                Py_XDECREF(input);
                Py_XDECREF(saveme);
                Py_XDECREF(tmp);
            }
        }
    }
    return ret;
}

long typeHelper_zVARs_(PyObject *tokens) {
    return typeHelper_rVARs_(tokens);
}

/* Look up the size of the datatype of the current rVar.
 * Requires no tokens. */
long typeHelper_rVAR_(PyObject *tokens) {
    return getToken(rVAR_DATATYPE_);
}

long typeHelper_zVAR_(PyObject *tokens) {
    return getToken(zVAR_DATATYPE_);
}

/* Look up how many dimensions rVariables in the current
 * CDF have for help in allocating arrays to contain
 * something which is per-dimension.
 * Requires no tokens. */
long helper_GET_rVARs_NUMDIMS_(PyObject *tokens) {
    return getToken(rVARs_NUMDIMS_);
}

/* Look up how many dimensions the currently selected
 * zVariable has for help in allocating arrays to contain
 * something which is per-dimension.
 * Requires no tokens. */
long helper_GET_zVAR_NUMDIMS_(PyObject *tokens) {
    return getToken(zVAR_NUMDIMS_);
}

/* Look up how the type of the currently selected
 * gAttribute for help in allocating memory to contain
 * the value the caller is about to retrieve.
 * Requires no tokens. */
long helper_GET_gENTRY_DATATYPE_(PyObject *tokens) {
    return typeConventionHelper(gENTRY_DATATYPE_, gENTRY_NUMELEMS_);
}

/* Look up how the type of the currently selected
 * rAttribute for help in allocating memory to contain
 * the value the caller is about to retrieve.
 * Requires no tokens. */
long helper_GET_rENTRY_DATATYPE_(PyObject *tokens) {
    return typeConventionHelper(rENTRY_DATATYPE_, rENTRY_NUMELEMS_);
}

/* Look up how the type of the currently selected
 * zAttribute for help in allocating memory to contain
 * the value the caller is about to retrieve.
 * Requires no tokens. */
long helper_GET_zENTRY_DATATYPE_(PyObject *tokens) {
    return typeConventionHelper(zENTRY_DATATYPE_, zENTRY_NUMELEMS_);
}



PyObject *tokenFormat_x_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    if (check(CDFlib(one, two, NULL_))) {
        return Py_None;
    }
    return NULL;
}

PyObject *tokenFormat_l_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    long in_1;
    if (PyArg_ParseTuple(tokens, "l", &in_1)) {
        if (check(CDFlib(one, two, in_1, NULL_))) {
            return Py_None;
        }
    }
    return NULL;
}

PyObject *tokenFormat_L_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    PyObject *in_1 = NULL;
    if (PyArg_ParseTuple(tokens, "O", &in_1)) {
        long *conv_1 = alloc(allocatedArrayFromOwnedPythonSequence(in_1));
        if (conv_1 != NULL) {
            if (check(CDFlib(one, two, conv_1, NULL_))) {
                free(conv_1);
                return Py_None;
            }
            free(conv_1);
        }
    }
    return NULL;
}

PyObject *tokenFormat_p_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    return tokenFormat_l_x(one, two, tokens, helper);
}

PyObject *tokenFormat_s_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    char *in_1 = NULL;
    if (PyArg_ParseTuple(tokens, "s", &in_1)) {
        if (check(CDFlib(one, two, in_1, NULL_))) {
            return Py_None;
        }
    }
    return NULL;
}

PyObject *tokenFormat_ll_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    long in_1;
    long in_2;
    if (PyArg_ParseTuple(tokens, "ll", &in_1, &in_2)) {
        if (check(CDFlib(one, two, in_1, in_2, NULL_))) {
            return Py_None;
        }
    }
    return NULL;
}

PyObject *tokenFormat_lL_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    long in_1;
    PyObject *in_2 = NULL;
    if (PyArg_ParseTuple(tokens, "lO", &in_1, &in_2)) {
        long *conv_2 = alloc(allocatedArrayFromOwnedPythonSequence(in_2));
        if (conv_2 != NULL) {
            if (check(CDFlib(one, two, in_1, conv_2, NULL_))) {
                free(conv_2);
                return Py_None;
            }
            free(conv_2);
        }
    }
    return NULL;
}

PyObject *tokenFormat_llV_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    long in_1;
    long in_2;
    PyObject *in_3 = NULL;
    if (PyArg_ParseTuple(tokens, "llO", &in_1, &in_2, &in_3)) {
        if (helper != NULL) {
            long type = helper(tokens);
            void *conv_3 = rebinFromPythonToC(in_3, type);
            if (conv_3 != NULL) {
                if (check(CDFlib(one, two, in_1, in_2, conv_3, NULL_))) {
                    free(conv_3);
                    return Py_None;
                }
                free(conv_3);
            }
        }
    }
    return NULL;
}

PyObject *tokenFormat_lLV_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    long in_1;
    PyObject *in_2 = NULL;
    PyObject *in_3 = NULL;
    if (PyArg_ParseTuple(tokens, "lOO", &in_1, &in_2, &in_3)) {
        long *conv_2 = alloc(allocatedArrayFromOwnedPythonSequence(in_2));
        if (conv_2 != NULL) {
            if (helper != NULL) {
                long type = helper(tokens);
                void *conv_3 = rebinFromPythonToC(in_3, type);
                if (conv_3 != NULL) {
                    if (check(CDFlib(one, two, in_1, conv_2, conv_3, NULL_))) {
                        free(conv_2);
                        free(conv_3);
                        return Py_None;
                    }
                    free(conv_3);
                }
            }
            free(conv_2);
        }
    }
    return NULL;
}

PyObject *tokenFormat_x_l(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    long out_1;
    if (check(CDFlib(one, two, &out_1, NULL_))) {
        return Py_BuildValue("(l)", out_1);
    }
    return NULL;
}

PyObject *tokenFormat_x_L(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    if (helper != NULL) {
        long len = helper(tokens);
        long *out_1 = alloc(calloc(sizeof(long), len));
        if ((out_1 != NULL) || (len == 0)) {
            if (check(CDFlib(one, two, out_1, NULL_))) {
                /* Convert long array list into Python list. */
                PyObject *conv_1 = ownedPythonListFromArray((void *)out_1, len, CDF_INT4);
                free(out_1);
                return Py_BuildValue("(O)", conv_1);
            }
        }
        free(out_1);
    }
    return NULL;
}

PyObject *tokenFormat_x_v(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    if (helper != NULL) {
        long type = helper(tokens);
        if (type < 0 || type == CDF_CHAR) {
            /* String handling has different ideas of "size". */
            return tokenFormat_x_s(one, two, tokens, helper);
        } else {
            long size = getSize(type);

            if (size > 0) {
                void *out_1 = alloc(calloc(size, 1));
                if (out_1 != NULL) {
                    if (check(CDFlib(one, two, out_1, NULL_))) {
                        PyObject *conv_1 = castFromCdfToPython(type, out_1);
                        if (conv_1 != NULL) {
                            PyObject *ret = Py_BuildValue("(O)", conv_1);
                            free(out_1);
                            Py_XDECREF(conv_1);
                            return ret;
                        }
                    }
                    free(out_1);
                }
            }
        }
    }
    return NULL;
}

PyObject *tokenFormat_x_V(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    if (helper != NULL) {
        long type = helper(tokens);
        if (type < 0) {
            return tokenFormat_x_s(one, two, tokens, helper);
        } else {
            long len = type % TYPE_MOD;
            type -= len;
            type /= TYPE_MOD;
            if (len > 0) {
                long size = getSize(type);
                void *out_1 = alloc(calloc(size, len));
                if (out_1 != NULL) {
                    if (check(CDFlib(one, two, out_1, NULL_))) {
                        /* Convert array into Python list. */
                        PyObject *conv_1
                          = ownedPythonListFromArray(out_1, len, type);
                        free(out_1);
                        return Py_BuildValue("(O)", conv_1);
                    }
                    free(out_1);
                }
            }
        }
    }
    return NULL;
}

PyObject *tokenFormat_x_c(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    char *out_1[2];
    out_1[1] = '\0';
    if (check(CDFlib(one, two, out_1, NULL_))) {
        return Py_BuildValue("(s)", out_1);
    }
    return NULL;
}

PyObject *tokenFormat_x_s(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    char out_1[MAX_OUTPUT_STRING_LEN];
    if (check(CDFlib(one, two, &out_1, NULL_))) {
        if (helper != NULL) {
            long size = abs(helper(tokens));
            out_1[size] = '\0';
        }
        return Py_BuildValue("(s)", out_1);
    }
    return NULL;
}

PyObject *tokenFormat_x_lL(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    if (helper != NULL) {
        long len = helper(tokens);
        if (len > 0) {
            long *out_2 = alloc(calloc(sizeof(long), len));
            if (out_2 != NULL) {
                long out_1;
                if (check(CDFlib(one, two, &out_1, out_2, NULL_))) {
                    /* Convert long array list into Python list. */
                    PyObject *conv_2 = ownedPythonListFromArray(
                      (void *)out_2, len, CDF_INT4);
                    free(out_2);
                    return Py_BuildValue("(lO)", out_1, conv_2);
                }
                free(out_2);
            }
        }
    }
    return NULL;
}

PyObject *tokenFormat_l_l(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    long in_1;
    long out_1;
    if (PyArg_ParseTuple(tokens, "l", &in_1)) {
        if (check(CDFlib(one, two, in_1, &out_1, NULL_))) {
            return Py_BuildValue("(l)", out_1);
        }
    }
    return NULL;
}

PyObject *tokenFormat_lL_v(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    long in_1;
    PyObject *in_2 = NULL;
    long out_1;
    if (PyArg_ParseTuple(tokens, "lO", &in_1, &in_2)) {
        /* TODO Determine the type of value, instead of just using
         * longs. */
        long *conv_2 = alloc(allocatedArrayFromOwnedPythonSequence(in_2));
        if (conv_2 != NULL) {
            if (check(CDFlib(one, two, in_1, conv_2, &out_1, NULL_))) {
                free(conv_2);
                return Py_BuildValue("(l)", out_1);
            }
            free(conv_2);
        }
    }
    return NULL;
}

PyObject *tokenFormat_s_l(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    char *in_1 = NULL;
    long out_1;
    if (PyArg_ParseTuple(tokens, "s", &in_1)) {
        if (check(CDFlib(one, two, in_1, &out_1, NULL_))) {
            return Py_BuildValue("(l)", out_1);
        }
    }
    return NULL;
}

PyObject *tokenFormat_sl_l(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    char *in_1 = NULL;
    long in_2;
    long out_1;
    if (PyArg_ParseTuple(tokens, "sl", &in_1, &in_2)) {
        if (check(CDFlib(one, two, in_1, in_2, &out_1, NULL_))) {
            return Py_BuildValue("(l)", out_1);
        }
    }
    return NULL;
}

PyObject *tokenFormat_slL_l(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    char *in_1 = NULL;
    long in_2;
    PyObject *in_3 = NULL;
    long out_1;
    if (PyArg_ParseTuple(tokens, "slO", &in_1, &in_2, &in_3)) {
        long *conv_3 = alloc(allocatedArrayFromOwnedPythonSequence(in_3));
        if (conv_3 != NULL) {
            if (check(CDFlib(one, two, in_1, in_2, conv_3, &out_1, NULL_))) {
                free(conv_3);
                return Py_BuildValue("(l)", out_1);
            }
            free(conv_3);
        }
    }
    return NULL;
}

PyObject *tokenFormat_slllL_l(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    char *in_1 = NULL;
    long in_2;
    long in_3;
    long in_4;
    PyObject *in_5 = NULL;
    long out_1;
    if (PyArg_ParseTuple(tokens, "slllO", &in_1, &in_2, &in_3, &in_4, &in_5)) {
        long *conv_5 = alloc(allocatedArrayFromOwnedPythonSequence(in_5));
        if (conv_5 != NULL) {
            if (check(CDFlib(one, two, in_1, in_2, in_3, in_4, conv_5,
                &out_1, NULL_))) {
                free(conv_5);
                return Py_BuildValue("(l)", out_1);
            }
            free(conv_5);
        }
    }
    return NULL;
}

PyObject *tokenFormat_slllLlL_l(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    char *in_1 = NULL;
    long in_2;
    long in_3;
    long in_4;
    PyObject *in_5 = NULL;
    long in_6;
    PyObject *in_7 = NULL;
    long out_1;
    if (PyArg_ParseTuple(tokens, "slllOlO", &in_1, &in_2, &in_3, &in_4, &in_5,
        &in_6, &in_7)) {
        long *conv_5 = alloc(allocatedArrayFromOwnedPythonSequence(in_5));
        if (conv_5 != NULL) {
            long *conv_7 = alloc(allocatedArrayFromOwnedPythonSequence(in_7));
            if (conv_7 != NULL) {
                if (check(CDFlib(one, two, in_1, in_2, in_3, in_4, conv_5,
                    in_6, conv_7, &out_1, NULL_))) {
                    free(conv_5);
                    free(conv_7);
                    return Py_BuildValue("(l)", out_1);
                }
                free(conv_7);
            }
            free(conv_5);
        }
    }
    return NULL;
}

/* The input value needs to be of the type of the current rVariable
 * selection. */
PyObject *tokenCustom_rVAR_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    PyObject *in_1 = PyTuple_GetItem(tokens, 0);
    void *conv_1 = rebinFromPythonToC(in_1, getToken(rVAR_DATATYPE_));
    if (conv_1 != NULL) {
        if (check(CDFlib(one, two, conv_1, NULL_))) {
            free(conv_1);
            return Py_None;
        }
        free(conv_1);
    }

    return NULL;
}

/* The input value needs to be of the type of the current zVariable
 * selection. */
PyObject *tokenCustom_zVAR_x(long one, long two, PyObject *tokens,
    long (*helper)(PyObject *)) {
    PyObject *in_1 = PyTuple_GetItem(tokens, 0);
    void *conv_1 = rebinFromPythonToC(in_1, getToken(zVAR_DATATYPE_));
    if (conv_1 != NULL) {
        if (check(CDFlib(one, two, conv_1, NULL_))) {
            free(conv_1);
            return Py_None;
        }
        free(conv_1);
    }
    return NULL;
}



PyObject *CdfFirstTierTokenHandler(
    PyObject *tokens,
    CdfFirstTierToken *tokenTable) {
    /* The return values. */
    PyObject *list = PyList_New(0);

    /* Utility variable. */
    long tokenCounter = 0;
    CdfFirstTierToken *match = NULL;
    long token = NULL_;

    if (!PySequence_Check(tokens)) {
        return NULL;
    }
    /* Token processing.  It is important to understand how the CDFlib
     * function handles tokens in order for us to properly handle ours.
     * The CDFlib function has a handful of "first tier" tokens which
     * indicate major actions to be taken.  Each of these actions accepts
     * some amount of arguments, but it is difficult to predict how
     * many in advance of processing.
     *
     * A valid sequence of tokens to CDFlib consists of at least one and
     * up to arbitrarily many valid first-tier phrases followed by the
     * special NULL_ token.  Since null termination is not pythonic in
     * the least, I have made the NULL_ token optional in this implementation.
     *
     * A valid first-tier phrase consists of a first-tier token followed
     * by an arbitrary number of valid second-tier phrases.  Note that
     * the first-tier token need not be repeated at the start of each valid
     * second-tier phrase; the last first-tier token will be assumed.  Also
     * note that each first-tier token accepts different second-tier tokens,
     * and that a second-tier token may mean different things when used
     * with different first-tier tokens.
     *
     * A valid second-tier phrase can take a variety of forms, but generally
     * consists of a valid second tier token followed by zero or more
     * arguments.  The number of arguments will always be the same for each
     * first-tier + second-tier token combination (no variation in number
     * of arguments).  The form of the arguments is very idiosyncratic and
     * the documentation should be consulted.  (The C reference manual for
     * the CDF source distribution is informative as it was the basis for
     * this implementation).
     *
     * So a call to this code might take the form
     * tuple = cdf.internal.CDFlib(
     *     cdf.internal.OPEN_,        # First-tier token
     *         cdf.internal.CDF_,     # Second-tier token
     *             "foo")               # Argument.  A string in this case.
     * 
     * However, the following would also be valid (by applying the last-seen
     * first-tier token to a second-tier phrase):
     * tuple = cdf.internal.CDFlib(
     *     cdf.internal.OPEN_,        # First-tier token
     *         cdf.internal.CDF_,     # Second-tier token
     *             "foo",               # Argument
     *         cdf.internal.CDF_,     # Second-tier token
     *             "bar")               # Argument
     */
    for ( ; tokenCounter < PySequence_Size(tokens); tokenCounter++) {
        PyObject *arg = PySequence_GetItem(tokens, tokenCounter);
        long skipCurrentToken = 0;
        long lastToken = token;
        if (PyLong_Check(arg) || PyInt_Check(arg)) {
            /* Locate the new token within token table. */
            CdfFirstTierToken *tmp = tokenTable;
            token = PyLong_AsLong(arg);
            while (tmp->valid) {
                if (tmp->token == token) {
                    match = tmp;
                    skipCurrentToken = 1;
                    break;
                } else {
                    tmp++;
                }
            }
            tmp = NULL;
        }
        if (!skipCurrentToken) {
            token = lastToken;
        }
        
        /* If the new token is associated with an action, that means
         * that any tokens we have accumulated up to this point should
         * be a coherent action.  Slice off all the tokens we've
         * already seen and not yet processed and use the appropriate
         * function to process them. */
        if (match != NULL) {
            if (match->function != NULL) {
                PyObject *output = NULL;
                long tokenOffset = tokenCounter + skipCurrentToken;
                output = match->function(token, tokens, &tokenOffset);
                if ((output != NULL) && PySequence_Check(output)) {
                    long retCounter;
                    for (retCounter = 0; 
                        retCounter < PySequence_Size(output);
                        retCounter++) {
                        /* Borrow a reference from the tuple. */
                        PyObject *item = PySequence_GetItem(
                            output, retCounter);
                        /* Up the refcount so we now own it. */
                        Py_IncRef(item);
                        /* Pass ownership to the list. */
                        PyList_Append(list, item);
                    }
                    /* No more use for the returned tuple. */
                    Py_DecRef(output);
                }
                /* Read back how many tokens were used by this code.
                 * Subtract one to correct for the impending increment
                 * which this for loop will perform. */
                tokenCounter = tokenOffset - 1;
            } else {
                /* No function pointer?  No problem!  You're probably
                 * just the NULL_ token and we're going to ignore you.
                 * There's nothing special to be done with the NULL_
                 * token since this probably means we're at the end of
                 * the token tuple anyway and we're just going to exit
                 * the loop after this iteration and start building the
                 * return tuple. */
            }
        } else {
            printf("Unrecognized first-tier token %ld.\n", token);
        }
        /* Note that we allow match to roll over into the next loop
         * iteration because we might not have another first-tier
         * token; possibly we have multiple second-tier phrases which
         * should be assumed to use the same first-tier token. */
    }

    if (PyErr_Occurred() != NULL) {
        Py_XDECREF(list);
        return NULL;
    } else {
        /* Construct the final return tuple, which is composed of the
         * aggregate status (the first error code to occur, or the
         * last warning to occur, or the last informational message
         * to occur, or CDF_OK), followed by the output of any tokens
         * in the order they were received.  It is up to the caller to
         * anticipate how much output they'll be receiving and what to do
         * with it. */
        long i;
        PyObject *tuple = NULL;
        /* First item is for status. */
        long len = PyList_Size(list);
        tuple = PyTuple_New(len);
        for (i = 0; i < len; i++) {
            PyObject *item = PyList_GetItem(list, i);
            Py_IncRef(item);
            PyTuple_SetItem(tuple, i, item);
        }
        Py_DecRef(list);
        return tuple;
    }
}

PyObject *CdfSecondTierTokenHandler(
    long one, PyObject *tokens, long *tokenOffset,
    CdfSecondTierToken *tokenTable) {

    if (!PySequence_Check(tokens)) {
        return NULL;
    }
    PyObject *arg = PySequence_GetItem(tokens, *tokenOffset);
    if (PyLong_Check(arg) || PyInt_Check(arg)) {
        /* Locate the new token within token table. */
        CdfSecondTierToken *tmp = tokenTable;
        long token = PyLong_AsLong(arg);
        CdfSecondTierToken *match = NULL;
        while (tmp->valid) {
            if (tmp->token == token) {
                match = tmp;
                break;
            } else {
                tmp++;
            }
        }
        tmp = NULL;

        if (match != NULL) {
            if (match->function != NULL) {
                /* If the new token is associated with an action, that means
                 * that any tokens we have accumulated up to this point should
                 * be a coherent action.  Slice off all the tokens we've
                 * already seen and not yet processed and use the appropriate
                 * function to process them. */
                PyObject *output = NULL;
                PyObject *input = NULL;
                long low = *tokenOffset + 1;
                long high = low + match->argumentsRequired;
                if (low < high) {
                    input = PySequence_GetSlice(tokens, low, high);
                    output = match->function(one, token, input, match->helper);
                    Py_DECREF(input);
                } else {
                    output = match->function(one, token, NULL, match->helper);
                }
                /* By setting the address of this variable we are effectively
                 * returning the number of tokens we peeled off of the token
                 * tuple so that the higher-level processing code knows they've
                 * been read and dealt with.  At the higher level they iterate
                 * over all tokens, and this will jump the iteration ahead. */
                *tokenOffset = high;

                if (PyErr_Occurred() != NULL) {
                    Py_XDECREF(output);
                    return NULL;
                } else {
                    return output;
                }
            }
        } else {
            printf("Unrecognized second-tier token %ld "
                "for first-tier token %ld.\n", token, one);
        }
    }
    return Py_None;
}



/* Utility function definitions. */
long *
allocatedArrayFromOwnedPythonSequence(PyObject *sequence) {
    long i;
    long len;
    long *array = NULL;
    PyObject *item = NULL;

    if ((sequence != NULL) && PySequence_Check(sequence)) {
        len = PySequence_Size(sequence);
        if (len > 0) {
            array = (long *)calloc(len, sizeof(long));
            if (array != NULL) {
                for (i = 0; i < len; i++) {
                    item = PySequence_GetItem(sequence, i);
                    if (item != NULL) {
                        array[i] = PyLong_AsLong(item);
                    } else {
                        free(array);
                        return NULL;
                    }
                }
                return array;
            }
        } else if (len == 0) {
            /* Since Python lists can be empty but an empty C list looks
             * like a null pointer which looks like a failure, we'll
             * make a dummy list.  This works on the theory that any C
             * list you'll be passing will have a separate length
             * argument which governs the handling of the list.  If this
             * is not the case, you should not be using this function. */
            return (long *)calloc(1, sizeof(long));
        }
    }
    return NULL;
}

PyObject *
ownedPythonListFromArray(void *array, long len, long type) {
    if ((array != NULL) || (len == 0)) {
        long size = getSize(type);
        long i;
        PyObject *list = PyList_New(len);
        if (list == NULL) {
            printf("Failed to create new Python list.\n");
            return NULL;
        }
        for (i = 0; i < len; i++) {
            PyObject *value = castFromCdfToPython(type, array + (i * size));
            if (value != NULL) {
                PyList_SetItem(list, i, value);
            } else {
                Py_DECREF(list);
                return NULL;
            }
        }
        return list;
    }
    printf("Not enough information to convert C array into python list.\n");
    return NULL;
}

void **
arrayOfArrayPointers(long count) {
    if (count > 0) {
        void **ret = (void **)calloc(count, sizeof(void *));

        if (ret == NULL) {
            printf("Failed to allocate memory for void pointer "
                "array of size %ld.\n", count);
            return NULL;
        }
        return ret;
    }
    return NULL;
}

long *arrayOfLongs(long count) {
    if (count > 0) {
        long *ret = (long *)calloc(count, sizeof(long));

        if (ret == NULL) {
            printf("Failed to allocate memory for long array "
                "of size %ld.\n", count);
            return NULL;
        }
        return ret;
    }
    return NULL;
}

void **
multiDimensionalArray(long *dims, long count) {
    long i;

    if (dims != NULL) {
        if (dims[0] > 0) {
            if (count > 1) {
                void **level = arrayOfArrayPointers(dims[0]);
                if (level == NULL) {
                    printf("Failed to allocate memory for array dimension.\n");
                    return NULL;
                }
                for (i = 0; i < dims[0]; i++) {
                    level[i] = multiDimensionalArray(
                        (long *)(&(dims[1])), (count - 1));
                }
                return level;
            } else {
                return (void **)arrayOfLongs(dims[0]);
            }
        }
    }
    return NULL;
}

void
cleanupMultiDimensionalArray(void **array, long *dims, long count) {
    long i;

    if ((array != NULL) && (dims != NULL)) {
        if (count > 1) {
            for (i = 0; i < dims[0]; i++) {
                cleanupMultiDimensionalArray(
                    (void **)(&(array[i])), (long *)(&(dims[1])), (count - 1));
            }
            free(array);
        } else {
            free((long *)array);
        }
    }
}

PyObject *
ownedPythonListOfListsFromArray(void **array, long *dims, long count) {
    PyObject *list = NULL;
    PyObject *tmp = NULL;
    long i;

    if ((array != NULL) && (dims != NULL)) {
        if (count > 1) {
            list = PyList_New(dims[0]);
            if (list != NULL) {
                for (i = 0; i < dims[0]; i++) {
                    tmp = ownedPythonListOfListsFromArray(
                        (void *)(&(array[i])), (long *)(&(dims[1])), (count - 1));
                    if (tmp != NULL) {
                        PyList_SetItem(list, i, tmp);
                    } else {
                        Py_DECREF(list);
                        return NULL;
                    }
                }
                return list;
            } else {
                printf("Failed to allocate new Python list.\n");
                return NULL;
            }
        } else {
            return ownedPythonListFromArray((void *)array, dims[0], CDF_INT4);
        }
    }
    printf("Not enough information to generate new Python list.\n");
    return NULL;
}
