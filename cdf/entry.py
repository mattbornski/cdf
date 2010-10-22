# Stock python modules

# cdf extension modules
import interface as cdf
import internal
import typing

# The principal advantage of having an object representing an entry is that
# it encapsulates the storage of the data along with the typing of the
# data.
# TODO We only throw errors about unwritable types when writing them.  It
# would be better to throw the errors when the user sets an unwritable type.
class entry:
    def __init__(self, value, simple = False):
        self._data = None
        self._cdfType = None
        self._numElements = None
        if self._type(value, simple):
            self._data = value
    def _type(self, value, simple = True):
        if isinstance(value, str) or isinstance(value, unicode):
            self._cdfType = internal.CDF_CHAR
            self._numElements = len(value)
        elif isinstance(value, int) or isinstance(value, long):
            self._cdfType = internal.CDF_INT4
            self._numElements = 1
        elif isinstance(value, float):
            self._cdfType = internal.CDF_REAL8
            self._numElements = 1
        elif isinstance(value, tuple):
            oldType = None
            for item in value:
                # Every item in a tuple must have the same type.
                newType = self._type(item)
                if newType[1] != 1:
                    # Only simple types, and non-string types,
                    # inside tuples.
                    # This is a CoherenceError because the type would be
                    # okay by itself but disagrees with other types which
                    # it is implicitly tied to.
                    raise cdf.CoherenceError
                newType = newType[0]
                if oldType is None:
                    oldType = newType
                elif oldType != newType:
                    # In the case of type disagreement, use the more forgiving
                    # type and size combo.
                    # In particular, if one is signed and one is unsigned, we
                    # must use signed.  If one is integer and one is floating
                    # point, we must use floating point.
                    joinedType = typing.joinCdfType(oldType, newType)
                    if joinedType is not None:
                        oldType = joinedType
                    else:
                        # This is a CoherenceError because the type would be
                        # okay by itself but disagrees with other types which
                        # it is implicitly tied to.
                        raise cdf.CoherenceError
            if oldType is not None:
                self._cdfType = oldType
                self._numElements = len(value)
            else:
                # This is a ValueError because you didn't actually give
                # me a value.  It's an empty list or something.
                raise ValueError
        # The simple flag prohibits lists.  Only global attributes can contain
        # multiple entries.
        elif isinstance(value, list) and not simple:
            # Unlike tuples, items in lists can be of different types
            types = []
            nums = []
            for item in value:
                try:
                    (t, n) = self._type(item)
                    types.append(t)
                    nums.append(n)
                except TypeError:
                    raise cdf.CoherenceError
            self._cdfType = types
            self._numElements = nums
        else:
            print 'Unknown type for data'
            print value
            print value.__class__
            print type(value)
            self._cdfType = None
            self._numElements = None
            # This is a ValueError because this value would never be
            # okay.
            raise ValueError
        return (self._cdfType, self._numElements)
    def _write(self, num1, token1, num2, token2, type, num3, value):
        internal.CDFlib(
            internal.SELECT_,
                internal.ATTR_,
                    num1,
                token1,
                    num2,
            internal.PUT_,
                token2,
                    type,
                    num3,
                    value)
    def write(self, token1, token2, attrNum, entryNum = None):
        if entryNum is None:
            data = self._data
            types = self._cdfType
            nums = self._numElements
            if not isinstance(data, list):
                data = [self._data]
                types = [self._cdfType]
                nums = [self._numElements]
            for i in xrange(0, len(data)):
                self._write(attrNum, token1, i, token2,
                  types[i], nums[i], data[i])
        else:
            self._write(attrNum, token1, entryNum, token2,
              self._cdfType, self._numElements, self._data)
