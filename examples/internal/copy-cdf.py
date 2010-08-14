#!/usr/bin/env python

import numpy
import os.path
import sys
import cdf.internal


def copy(filename_from, filename_to):
    (id_from, ) = cdf.internal.CDFlib(
        cdf.internal.OPEN_,
            cdf.internal.CDF_,
                filename_from)
    cdf.internal.CDFlib(
        cdf.internal.SELECT_,
            cdf.internal.CDF_,
                id_from)
    r = {}
    z = {}
    (rVars, zVars) = cdf.internal.CDFlib(
        cdf.internal.GET_,
            cdf.internal.CDF_NUMrVARS_,
            cdf.internal.CDF_NUMzVARS_)
    (rVarDimSizes, ) = cdf.internal.CDFlib(
        cdf.internal.GET_,
            cdf.internal.rVARs_DIMSIZES_)
    rVarNumDims = len(rVarDimSizes)
    for var in xrange(0, rVars):
        (name, numRecords, type, elements, recVary, dimVary) \
            = cdf.internal.CDFlib(
            cdf.internal.SELECT_,
                cdf.internal.rVAR_,
                    var,
            cdf.internal.GET_,
                cdf.internal.rVAR_NAME_,
                cdf.internal.rVAR_MAXREC_,
                cdf.internal.rVAR_DATATYPE_,
                cdf.internal.rVAR_NUMELEMS_,
                cdf.internal.rVAR_RECVARY_,
                cdf.internal.rVAR_DIMVARYS_)
        values = []
        for record in xrange(0, numRecords):
            value = numpy.zeros(rVarDimSizes)
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
            values.append(value)
        cdf.internal.CDFlib(
            cdf.internal.CLOSE_,
                cdf.internal.rVAR_)
        r[name] = (values, type, elements, recVary, dimVary)
    for var in xrange(0, zVars):
        (name, dimSizes, numRecords) = cdf.internal.CDFlib(
            cdf.internal.SELECT_,
                cdf.internal.zVAR_,
                    var,
            cdf.internal.GET_,
                cdf.internal.zVAR_NAME_,
                cdf.internal.zVAR_DIMSIZES_,
                cdf.internal.zVAR_MAXREC_)
        values = []
        for record in xrange(0, numRecords):
            value = numpy.zeros(dimSizes)
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
            values.append(value)
        cdf.internal.CDFlib(
            cdf.internal.CLOSE_,
                cdf.internal.zVAR_)
        z[name] = values
    cdf.internal.CDFlib(
        cdf.internal.CLOSE_,
            cdf.internal.CDF_)

    id_to = None
    if os.path.exists(filename_to):
        (id_to, ) = cdf.internal.CDFlib(
            cdf.internal.OPEN_,
                cdf.internal.CDF_,
                    filename_to)
    else:
        # Make sure to strip the '.cdf' from the filename as the CDF
        # library doesn't handle that well.
        (filename, ext) = os.path.splitext(filename_to)
        (id_to, ) = cdf.internal.CDFlib(
            cdf.internal.CREATE_,
                cdf.internal.CDF_,
                    filename,
                    rVarNumDims,
                    (rVarDimSizes if rVarNumDims > 0 else [0]))
    for name in r.keys():
        (values, type, elements, recVary, dimVary) = r[name]
        (num, ) = cdf.internal.CDFlib(
            cdf.internal.CREATE_,
                cdf.internal.rVAR_,
                    name,
                    type,
                    elements,
                    recVary,
                    dimVary)
        cdf.internal.CDFlib(
            cdf.internal.SELECT_,
                cdf.internal.rVAR_,
                    num,
            cdf.internal.PUT_,
                cdf.internal.rVAR_ALLOCATERECS_,
                len(values))
        for record in xrange(0, len(values)):
            for index, value in numpy.ndenumerate(values[record]):
                cdf.internal.CDFlib(
                    cdf.internal.SELECT_,
                        cdf.internal.rVARs_RECNUMBER_,
                            record,
                        cdf.internal.rVARs_DIMINDICES_,
                            index,
                    cdf.internal.PUT_,
                        cdf.internal.rVAR_DATA_,
                            value)
        cdf.internal.CDFlib(
            cdf.internal.CLOSE_,
                cdf.internal.rVAR_)
    for name in z.keys():
        pass
    cdf.internal.CDFlib(
        cdf.internal.CLOSE_,
            cdf.internal.CDF_)

if __name__ == '__main__':
    copy(sys.argv[1], sys.argv[2])
