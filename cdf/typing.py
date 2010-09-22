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
  numpy.string_:[numpy.string_],
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
    for dtype in args:
        if not isinstance(dtype, numpy.dtype):
            dtype = numpy.dtype(dtype)
        if ret is None:
            ret = dtype
        elif _typeContainsOther(dtype.type, ret.type) \
          and dtype.itemsize >= ret.itemsize:
            ret = dtype
        elif not _typeContainsOther(ret.type, dtype.type) \
          or dtype.itemsize > ret.itemsize:
            # There's not a simple inclusion here.  We might have to
            # upgrade to a third type which encapsulates both.
            candidates = []
            for type in _numpyTypeContains:
                if _typeContainsOther(type, ret.type) \
                  and _typeContainsOther(type, dtype.type):
                    candidates.append(type)
            if len(candidates) > 0:
                # TODO choose the type from the available candidates more
                # intelligently.
                ret = numpy.dtype(candidates[0])
            else:
                # Abort.
                ret = None
            break
    return ret

def joinCdfType(*args):
    # These are CDF types.
#    try:
        return _typeConversions[joinNumpyType(*[_typeConversions[type] for type in args]).type]
#    except KeyError:
#        return None

# Helper class for objects which must have the same type and dimensionality.
# In typing objects for CDF, it's important to note a few things.
# The type assigned in the file may or may not be authoritative.  If you
# are using the Pythonic interface of this CDF package, it is not - the
# types are assigned because the standard requires them, not because we
# wish to restrict the data to the type given.  If you're reading in
# somebody else's CDF file, painstakingly crafted by hand, they might very
# well be by design.  There is no good and well understood way to pass
# along the intent behind the typing, and so we assume that types set
# in the file are _not_ authoritative, and may be changed as needed
# to accommodate new data.
# If at any point during a session, however, the _user_ assigns a type
# explicitly, that is understood to be authoritative, for pretty clear
# reasons.
# Knowing the type of the data is important when:
# 1.) the underlying internal interface is reading the data from the file
#     and must allocate and manage storage properly
# 2.) the pythonic interface is attempting to warn the user of incompatible
#     data types in a set of objects which are supposed to be of the
#     same type and dimensionality.  TODO: warnings at time of assignment
#     are not yet implemented.
# 3.) the pythonic interface instructs the internal interface to allocate
#     storage for the set of data in preparation for writing the data to
#     disk.
# In the first case, we know the type by simply querying the file.  In the
# third case, it is easy enough to determine the type by polling all data,
# as we are already committed to an order n operation in the write itself.
# It's the second case that's trickiest, as it means we must maintain a
# constantly-updated understanding of which types we can and cannot use.
# We can do this by basing our initial type off of the type currently
# assigned in the file, and modifying this guess based on the deletions
# and assignments the user carries out over the course of the session.
class uniformCdfTyped:
    pass
