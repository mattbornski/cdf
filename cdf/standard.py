# cdf extension modules.
from internal import *

# Implemented as macros in cdf.h in the source distribution.
def CDFcreate(CDFname, numDims, dimSizes, encoding, majority):
    return CDFlib(
        CREATE_,
            CDF_,
                CDFname, numDims, dimSizes,
	    PUT_,
            CDF_ENCODING_,
                encoding,
	        CDF_MAJORITY_,
                majority,
	    NULL_)

def CDFopen(CDFname):
    return CDFlib(
        OPEN_,
            CDF_,
                CDFname,
	    NULL_)

def CDFdoc(id):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
	    GET_,
            CDF_VERSION_,
	        CDF_RELEASE_,
	        CDF_COPYRIGHT_,
	    NULL_)

def CDFinquire(id):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
	    GET_,
            rVARs_NUMDIMS_,
	        rVARs_DIMSIZES_,
	        CDF_ENCODING_,
	        CDF_MAJORITY_,
	        rVARs_MAXREC_,
	        CDF_NUMrVARS_,
	        CDF_NUMATTRS_,
	    NULL_)

def CDFclose(id):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
	    CLOSE_,
            CDF_,
	    NULL_)

def CDFdelete(id):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
	    DELETE_,
            CDF_,
	    NULL_)

def CDFerror(stat):
    return CDFlib(
        SELECT_,
            CDF_STATUS_,
                stat,
	    GET_,
            STATUS_TEXT_,
	    NULL_)

def CDFattrCreate(id, attrName, attrScope):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
	    CREATE_,
            ATTR_,
                attrName, attrScope,
	    NULL_)

def CDFattrRename(id, attrNum, attrName):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
		    ATTR_,
                attrNum,
	    PUT_,
            ATTR_NAME_,
                attrName,
    	NULL_)

def CDFvarCreate(id, varName, dataType, numElements, recVary, dimVarys, varNum):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
	    CREATE_,
            rVAR_,
                varName, dataType, numElements, recVary, dimVarys, varNum,
	    NULL_)

def CDFvarRename(id, varNum, varName):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
		    rVAR_,
                varNum,
	    PUT_,
            rVAR_NAME_,
                varName,
	    NULL_)

def CDFvarInquire(id, varN):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
		    rVAR_,
                varN,
	    GET_,
            rVAR_NAME_,
	        rVAR_DATATYPE_,
	        rVAR_NUMELEMS_,
	        rVAR_RECVARY_,
	        rVAR_DIMVARYS_,
	    NULL_)

def CDFvarGet(id, varNum, recNum, indices):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
		    rVAR_,
                varNum,
		    rVARs_RECNUMBER_,
                recNum,
		    rVARs_DIMINDICES_,
                indices,
	    GET_,
            rVAR_DATA_,
	    NULL_)

def CDFvarPut(id, varNum, recNum, indices, value):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
		    rVAR_,
                varNum,
		    rVARs_RECNUMBER_,
                recNum,
		    rVARs_DIMINDICES_,
                indices,
	    PUT_,
            rVAR_DATA_,
                value,
	    NULL_)

def CDFvarHyperGet(id, varN, recS, recC, recI, indices, counts, intervals):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
            rVAR_,
                varN,
		    rVARs_RECNUMBER_,
                recS,
		    rVARs_RECCOUNT_,
                recC,
		    rVARs_RECINTERVAL_,
                recI,
		    rVARs_DIMINDICES_,
                indices,
		    rVARs_DIMCOUNTS_,
                counts,
		    rVARs_DIMINTERVALS_,
                intervals,
	    GET_,
            rVAR_HYPERDATA_,
	    NULL_)

def CDFvarHyperPut(id, varN, recS, recC, recI, indices, counts, intervals, buff):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
            rVAR_,
                varN,
    		rVARs_RECNUMBER_,
                recS,
		    rVARs_RECCOUNT_,
                recC,
		    rVARs_RECINTERVAL_,
                recI,
		    rVARs_DIMINDICES_,
                indices,
		    rVARs_DIMCOUNTS_,
                counts,
		    rVARs_DIMINTERVALS_,
                intervals,
    	PUT_,
            rVAR_HYPERDATA_,
                buff,
	    NULL_)

def CDFvarClose(id,varNum):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
            rVAR_,
                varNum,
    	CLOSE_,
            rVAR_,
    	NULL_)



# Implemented as functions in cdf_c_if.c in the source distribution.
def CDFattrInquire(id, attrNum):
    (attrScope, ) = CDFlib(
        SELECT_,
            CDF_,
                id,
            ATTR_,
                attrNum,
        GET_,
            ATTR_SCOPE_,
        NULL_)
    (attrName, maxEntry) = CDFlib(
        SELECT_,
            CDF_,
                id,
        GET_,
            ATTR_NAME_,
            ATTR_MAXgENTRY_ if attrScope == GLOBAL_SCOPE else ATTR_MAXrENTRY_,
        NULL_)
    return (attrScope, attrName, maxEntry)

def CDFattrEntryInquire(id, attrNum, entryNum):
    (attrScope, ) = CDFlib(
        SELECT_,
            CDF_,
                id,
            ATTR_,
                attrNum,
        GET_,
            ATTR_SCOPE_,
        NULL_)
    (dataType, numElems) = CDFlib(
        SELECT_,
            CDF_,
                id,
            gENTRY_ if attrScope == GLOBAL_SCOPE else rENTRY_,
                entryNum,
        GET_,
            gENTRY_DATATYPE if attrScope == GLOBAL_SCOPE else rENTRY_DATATYPE,
            gENTRY_NUMELEMS if attrScope == GLOBAL_SCOPE else rENTRY_NUMELEMS,
        NULL_)
    return (attrScope, dataType, numElems)

def CDFattrPut(id, attrNum, entryNum, dataType, numElements, value):
    (attrScope, ) = CDFlib(
        SELECT_,
            CDF_,
                id,
            ATTR_,
                attrNum,
        GET_,
            ATTR_SCOPE_,
        NULL_)
    (dataType, numElems) = CDFlib(
        SELECT_,
            CDF_,
                id,
            gENTRY_ if attrScope == GLOBAL_SCOPE else rENTRY_,
                entryNum,
        PUT_,
            gENTRY_DATA_ if attrScope == GLOBAL_SCOPE else rENTRY_DATA_,
                dataType,
                numElems,
                value,
        NULL_)

def CDFattrGet(id, attrNum, entryNum, value):
    (attrScope, ) = CDFlib(
        SELECT_,
            CDF_,
                id,
            ATTR_,
                attrNum,
        GET_,
            ATTR_SCOPE_,
        NULL_)
    return CDFlib(
        SELECT_,
            CDF_,
                id,
            gENTRY_ if attrScope == GLOBAL_SCOPE else rENTRY_,
                entryNum,
        GET_,
            gENTRY_DATA_ if attrScope == GLOBAL_SCOPE else rENTRY_DATA_,
        NULL_)

def CDFattrNum(id, attrName):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
        GET_,
            ATTR_NUMBER_,
                attr_name,
        NULL_)

def CDFvarNum(id, varName):
    return CDFlib(
        SELECT_,
            CDF_,
                id,
        GET_,
            rVAR_NUMBER_,
                var_name,
        NULL_)
