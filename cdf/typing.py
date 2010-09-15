#!/usr/bin/env python

# Stock Python modules

# Third-party modules
import numpy

# cdf extension modules
from . import internal

# TODO Develop a numpy dtype that properly encodes the complexities and
# precision of CDF EPOCH and EPOCH16 types.
class epoch(numpy.float64):
    pass

class epoch16(epoch):
    pass

# Type conversion lookups
_typeConversions = {
  # NumPy to CDF
  numpy.byte:               internal.CDF_BYTE,
  numpy.int8:               internal.CDF_INT1,
  numpy.int16:              internal.CDF_INT2,
  numpy.int32:              internal.CDF_INT4,
  numpy.int64:              internal.CDF_REAL8,
  numpy.uint8:              internal.CDF_UINT1,
  numpy.uint16:             internal.CDF_UINT2,
  numpy.uint32:             internal.CDF_UINT4,
  numpy.float32:            internal.CDF_REAL4,
  numpy.float64:            internal.CDF_REAL8,
  epoch:                    internal.CDF_EPOCH,
  epoch16:                  internal.CDF_EPOCH16,
  numpy.string_:            internal.CDF_CHAR,
  # CDF to NumPy
  internal.CDF_BYTE:        numpy.byte,
  internal.CDF_INT1:        numpy.int8,
  internal.CDF_INT2:        numpy.int16,
  internal.CDF_INT4:        numpy.int32,
  internal.CDF_UINT1:       numpy.uint8,
  internal.CDF_UINT2:       numpy.uint16,
  internal.CDF_UINT4:       numpy.uint32,
  internal.CDF_REAL4:       numpy.float32,
  internal.CDF_REAL8:       numpy.float64,
  internal.CDF_FLOAT:       numpy.float32,
  internal.CDF_DOUBLE:      numpy.float64,
  internal.CDF_EPOCH:       epoch,
  internal.CDF_EPOCH16:     epoch16,
  internal.CDF_CHAR:        numpy.string_,
  internal.CDF_UCHAR:       numpy.string_,
}

_numpyTypeSizes = {
  numpy.byte:1,
  numpy.int8:1,
  numpy.int16:2,
  numpy.int32:4,
  numpy.int64:8,
  numpy.uint8:1,
  numpy.uint16:2,
  numpy.uint32:4,
  numpy.float32:4,
  numpy.float64:8,
}

# This is a naive flow which doesn't take into account precision.  It does
# take into account signing, however.
_numpyTypeContains = {
  numpy.byte:[],
  numpy.int8:[numpy.byte],
  numpy.int16:[numpy.int8, numpy.uint8],
  numpy.int32:[numpy.int16, numpy.uint16],
  numpy.int64:[numpy.int32, numpy.uint32],
  numpy.uint8:[numpy.byte],
  numpy.uint16:[numpy.uint8],
  numpy.uint32:[numpy.uint16],
  numpy.float32:[numpy.int64],
  numpy.float64:[numpy.float32],
}

def _typeContainsOther(one, two, memo = None):
    # Terminal case
    if one == two:
        return True
    # Recursive case
    else:
        if memo is None:
            memo = {}
            memo[one] = False
        for type in _numpyTypeContains[one]:
            if not type in memo:
                # Note that we are presently investigating this possibility.
                memo[type] = None
                # When we receive the actual answer, note that instead.
                memo[type] = _typeContainsOther(type, two, memo)
            if memo[type]:
                return True
        return False

def joinNumpyType(*args):
    ret = None
    for type in args:
        if ret is None or _typeContainsOther(type, ret):
            ret = type
        elif not _typeContainsOther(ret, type):
            # Abort.
            ret = None
            break
    return ret

def joinCdfType(*args):
    # These are CDF types.
    try:
        return _typeConversions[joinNumpyType(*[_typeConversions[type] for type in args])]
    except KeyError:
        return None
