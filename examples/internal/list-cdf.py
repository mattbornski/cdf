#!/usr/bin/env python

import numpy
import sys
import cdf.internal

# Type conversion lookups
_typeConversions = {
    # NumPy to CDF
    numpy.byte:         cdf.internal.CDF_BYTE,
    numpy.int8:         cdf.internal.CDF_INT1,
    numpy.int16:        cdf.internal.CDF_INT2,
    numpy.int32:        cdf.internal.CDF_INT4,
    numpy.int64:        cdf.internal.CDF_REAL8,
    numpy.uint8:        cdf.internal.CDF_UINT1,
    numpy.uint16:       cdf.internal.CDF_UINT2,
    numpy.uint32:       cdf.internal.CDF_UINT4,
    numpy.float32:      cdf.internal.CDF_REAL4,
    numpy.float64:      cdf.internal.CDF_REAL8,
    numpy.string_:      cdf.internal.CDF_CHAR,
    # CDF to NumPy
    cdf.internal.CDF_BYTE:  numpy.byte,
    cdf.internal.CDF_INT1:  numpy.int8,
    cdf.internal.CDF_INT2:  numpy.int16,
    cdf.internal.CDF_INT4:  numpy.int32,
    cdf.internal.CDF_UINT1: numpy.uint8,
    cdf.internal.CDF_UINT2: numpy.uint16,
    cdf.internal.CDF_UINT4: numpy.uint32,
    cdf.internal.CDF_REAL4: numpy.float32,
    cdf.internal.CDF_REAL8: numpy.float64,
    cdf.internal.CDF_CHAR:  numpy.string_
}

def list(filename):
    (id, ) = cdf.internal.CDFlib(
        cdf.internal.OPEN_,
            cdf.internal.CDF_,
                filename)
    cdf.internal.CDFlib(
        cdf.internal.SELECT_,
            cdf.internal.CDF_,
                id)
    (format, ) = cdf.internal.CDFlib(
        cdf.internal.GET_,
            cdf.internal.CDF_FORMAT_)
    monofile = (format == cdf.internal.SINGLE_FILE)
    (rVars, zVars) = cdf.internal.CDFlib(
        cdf.internal.GET_,
            cdf.internal.CDF_NUMrVARS_,
            cdf.internal.CDF_NUMzVARS_)
    print 'Archive "' + filename + '"'
    for var in xrange(0, rVars):
        (name, dimSizes, numRecords, cdfType, numElems) \
          = cdf.internal.CDFlib(
            cdf.internal.SELECT_,
                cdf.internal.rVAR_,
                    var,
            cdf.internal.GET_,
                cdf.internal.rVAR_NAME_,
                cdf.internal.rVARs_DIMSIZES_,
                cdf.internal.rVAR_MAXREC_,
                cdf.internal.rVAR_DATATYPE_,
                cdf.internal.rVAR_NUMELEMS_)
        numpyType = _typeConversions[cdfType]
        if numpyType == numpy.string_:
            numpyType = numpy.dtype('|S' + str(numElems))
        print ' + Variable "' + name + '"'
        values = []
        for record in xrange(0, numRecords + 1):
            value = numpy.zeros(dimSizes, dtype = numpyType)
            for index, zero in numpy.ndenumerate(value):
                (entry, ) = cdf.internal.CDFlib(
                    cdf.internal.SELECT_,
                        cdf.internal.rVARs_RECNUMBER_,
                            record,
                        cdf.internal.rVARs_DIMINDICES_,
                            index,
                    cdf.internal.GET_,
                        cdf.internal.rVAR_DATA_)
                value[index] = entry
            values.append(str(value))
        if not monofile:
            cdf.internal.CDFlib(
                cdf.internal.CLOSE_,
                    cdf.internal.rVAR_)
        print '   = ' + str(values)
    for var in xrange(0, zVars):
        (name, dimSizes, numRecords, cdfType, numElems) \
          = cdf.internal.CDFlib(
            cdf.internal.SELECT_,
                cdf.internal.zVAR_,
                    var,
            cdf.internal.GET_,
                cdf.internal.zVAR_NAME_,
                cdf.internal.zVAR_DIMSIZES_,
                cdf.internal.zVAR_MAXREC_,
                cdf.internal.zVAR_DATATYPE_,
                cdf.internal.zVAR_NUMELEMS_)
        numpyType = _typeConversions[cdfType]
        if numpyType == numpy.string_:
            numpyType = numpy.dtype('|S' + str(numElems))
        print ' + Variable "' + name + '"'
        values = []
        for record in xrange(0, numRecords + 1):
            value = numpy.zeros(dimSizes, dtype = numpyType)
            for index, zero in numpy.ndenumerate(value):
                (entry, ) = cdf.internal.CDFlib(
                    cdf.internal.SELECT_,
                        cdf.internal.zVAR_RECNUMBER_,
                            record,
                        cdf.internal.zVAR_DIMINDICES_,
                            index,
                    cdf.internal.GET_,
                        cdf.internal.zVAR_DATA_)
                value[index] = entry
            values.append(str(value))
        if not monofile:
            cdf.internal.CDFlib(
                cdf.internal.CLOSE_,
                    cdf.internal.zVAR_)
        print '   = ' + str(values)
    cdf.internal.CDFlib(
        cdf.internal.CLOSE_,
            cdf.internal.CDF_)

if __name__ == '__main__':
    for filename in sys.argv[1:]:
        list(filename)
