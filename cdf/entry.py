# Stock python modules

# cdf extension modules
import internal

# The principal advantage of having an object representing an entry is that
# it encapsulates the storage of the data along with the typing of the
# data.
class entry:
    def __init__(self, value):
        self._value = None
        self._cdfType = None
        self._numElements = None
        if value is not None:
            self.set(value)
    def set(self, value):
        if isinstance(value, list):
            if len(value) == 1:
                value = value[0]
        self._value = value
        self._numElements = 1
        if isinstance(self._value, str) or isinstance(self._value, unicode):
            self._cdfType = internal.CDF_CHAR
            self._numElements = len(self._value)
        elif isinstance(self._value, int):
            self._cdfType = internal.CDF_INT4
        elif isinstance(self._value, float):
            self._cdfType = internal.CDF_REAL8
        else:
            print 'Unknown type for data'
            print self._value
            print type(self._value)
            self._value = None
            self._cdfType = None
            self._numElements = None
