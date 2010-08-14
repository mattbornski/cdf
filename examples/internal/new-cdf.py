#!/usr/bin/env python

import os.path
import sys
import time
import cdf.internal


def new(filename):
    id = None
    if os.path.exists(filename):
        (id, ) = cdf.internal.CDFlib(
            cdf.internal.OPEN_,
                cdf.internal.CDF_,
                    filename)
    else:
        # Make sure to strip the '.cdf' from the filename as the CDF
        # library doesn't handle that well.
        (filename, ext) = os.path.splitext(filename)
        (id, ) = cdf.internal.CDFlib(
            cdf.internal.CREATE_,
                cdf.internal.CDF_,
                    filename,
                    0,
                    [0])
    cdf.internal.CDFlib(
        cdf.internal.SELECT_,
            cdf.internal.CDF_,
                id)
    (num, ) = cdf.internal.CDFlib(
        cdf.internal.CREATE_,
            cdf.internal.zVAR_,
                'foo',
                cdf.internal.CDF_REAL8,
                1,
                0,
                [0],
                cdf.internal.VARY,
                [cdf.internal.VARY])
    cdf.internal.CDFlib(
        cdf.internal.SELECT_,
            cdf.internal.zVAR_,
                num,
        cdf.internal.PUT_,
            cdf.internal.zVAR_ALLOCATERECS_,
                1,
        cdf.internal.SELECT_,
            cdf.internal.zVAR_RECNUMBER_,
                0,
        cdf.internal.PUT_,
            cdf.internal.zVAR_DATA_,
                time.time())
    cdf.internal.CDFlib(
        cdf.internal.CLOSE_,
            cdf.internal.CDF_)

if __name__ == '__main__':
    new(sys.argv[1])
