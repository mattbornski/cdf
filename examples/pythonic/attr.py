#!/usr/bin/env python

from cdf.internal import *
import os

if __name__ == '__main__':
    if os.path.exists('attr.cdf'):
        os.remove('attr.cdf')
    (id, ) = CDFlib(
        CREATE_,
            CDF_,
                'attr',
                0,
                [0])
    (attrNum, ) = CDFlib(
        CREATE_,
            ATTR_,
                'pi',
                GLOBAL_SCOPE)
    CDFlib(
        SELECT_,
            ATTR_,
                attrNum)
    CDFlib(
        SELECT_,
            gENTRY_,
                0)
    CDFlib(
        PUT_,
            gENTRY_DATA_,
                CDF_REAL8,
                1,
                [3.14])

    (fiveNum, ) = CDFlib(
        CREATE_,
            zVAR_,
                "five",
                CDF_UINT4,
                1,
                0,
                [0],
                VARY,
                [VARY])
    CDFlib(
        SELECT_,
            zVAR_,
                fiveNum,
        PUT_,
            zVAR_ALLOCATERECS_,
                1,
        SELECT_,
            zVAR_RECNUMBER_,
                0,
        PUT_,
            zVAR_DATA_,
                5)
    (eightNum, ) = CDFlib(
        CREATE_,
            zVAR_,
                "eight",
                CDF_UINT4,
                1,
                0,
                [0],
                VARY,
                [VARY])
    CDFlib(
        SELECT_,
            zVAR_,
                eightNum,
        PUT_,
            zVAR_ALLOCATERECS_,
                1,
        SELECT_,
            zVAR_RECNUMBER_,
                0,
        PUT_,
            zVAR_DATA_,
                8)
    (attrNum, ) = CDFlib(
        CREATE_,
            ATTR_,
                'units',
                VARIABLE_SCOPE)
    CDFlib(
        SELECT_,
            ATTR_,
                attrNum,
            zENTRY_,
                fiveNum,
        PUT_,
            zENTRY_DATA_,
                CDF_CHAR,
                5,
                'five',
        SELECT_,
            ATTR_,
                attrNum,
            zENTRY_,
                eightNum,
        PUT_,
            zENTRY_DATA_,
#                CDF_CHAR,
#                6,
#                'eight')
                CDF_REAL4,
                1,
                8.0808)
    CDFlib(
        CLOSE_,
            CDF_)
