# Stock python modules
import copy
import os
import threading
import time
import weakref

# Third-party modules
import numpy

# cdf extension modules
from . import internal
from . import attribute
from . import typing


# The underlying CDF library is not thread-safe.  Make our layer thread
# safe by requiring you to acquire 
_selection_lock = threading.RLock()

# Closure to ensure proper selection of CDF files.
class _selection(object):
    tokens = {}
    parents = []
    def __init__(self):
        global _selection_lock
        self._lock = _selection_lock
    def __enter__(self, *args, **kwargs):
        # Acquire the process-wide lock on CDF selection.  This is a 
        # re-entrant lock as long as the re-acquisition comes from the
        # same thread which currently holds it.
        self._lock.acquire()
        # Acquire any "parent" selections, for instance, the archive's
        # selection if we are a variable, or the variable's selection if
        # we are a record.
        for parent in self.parents:
            parent.__enter__(*args, **kwargs)
        if len(self.tokens.keys()) > 0:
            args = [internal.SELECT_]
            # Selection precedence is important.  Selecting something
            # without first selecting its context probably will cause
            # things to fail mysteriously.
            # For zVariable data records, precedence goes
            #  archive -> variable -> record
            # For rVariable data records, the archive must come first, but
            # the record number is global and does not depend on the
            # variable being selected, so you can select the record number
            # and the variable number independently.
            # For gAttributes, precedence goes archive -> attribute -> entry.
            # For vAttributes, archive must come first, entry must come last,
            # but you can select the attribute and the variable in either
            # order in the middle.
            # Using our inside knowledge of how the tokens are defined,
            # which might not be the best idea going forward but is super
            # convenient right now, we can see that the tokens which are
            # defined to be the lowest numerically should have the highest
            # precedence:
            #define CDF_                    1L
            #define rVARs_RECNUMBER_        29L
            #define rVAR_                   35L
            #define zVAR_                   57L
            #define zVAR_RECNUMBER_         79L
            #define ATTR_                   85L
            #define gENTRY_                 96L
            #define rENTRY_                 102L
            #define zENTRY_                 109L
            for token in sorted(self.tokens.keys()):
                args += [token, self.tokens[token]]
            internal.CDFlib(*args)
    def __exit__(self, *args, **kwargs):
        self._lock.release()

# Error when a coherence restriction of the CDF format is violated.
class CoherenceError(Exception):
    pass



class record(numpy.ndarray):
    # NumPy overrides
    # To understand the structure used here, I refer you to
    # the NumPy docs for subclassing, currently available at
    # http://docs.scipy.org/doc/numpy/user/basics.subclassing.html
    def __new__(cls, input_array = None, variable = None, num = None):
        # This flag will help guide the record as to whether or not
        # it needs to look up actual values from the archive on disk.
        placeholder = True
        epoch_type = False
        if variable is not None:
            obj = numpy.asarray(numpy.zeros(
                shape = variable._dimSizes,
                dtype = variable._dtype)).view(cls)
        else:
            coerce = None
            if input_array is not None:
                if not isinstance(input_array, numpy.ndarray):
                    # This is not an n-dimensional array.  Can we coerce it to
                    # be an n-dimensional array?
                    try:
                        placeholder = False
                        coerce = numpy.asarray(input_array)
                    except TypeError:
                        coerce = None
                        raise TypeError('Record values must be arrays ' \
                            + 'or coercable into arrays.')
                else:
                    coerce = input_array
            if not isinstance(coerce, numpy.ndarray):
                coerce = numpy.zeros(shape = ()).view(type = cls)
            # HACK TODO Force int32 for integer types.  Numpy defaults
            # to int64 when you assign plain boring integers to it on
            # 64 bit machines, but the type unification code I have in
            # place unfortunately unifies int64s to float64s because
            # that's the only 64 bit type supported in CDF.  I'm not
            # sure what issues this will cause yet, but I bet it won't
            # be pretty somewhere down the line.
            if coerce.dtype == numpy.int64:
                obj = numpy.asarray(coerce, dtype = numpy.int32).view(cls)
            # HACK TODO force epoch type from datetime.  It would be lovely
            # if we could just constrain the types of the numpy array
            # and make a datetime object coercable directly but we must
            # work around it here.
            elif coerce.dtype == numpy.object:
                obj = numpy.zeros(shape = coerce.shape, dtype = typing.epoch)
                for (index, value) in numpy.ndenumerate(coerce):
                    obj[index] = typing.epoch(value)
                obj = obj.view(cls)
                epoch_type = True
            else:
                obj = coerce.view(cls)
        obj._num = num
        obj._variable = variable
        obj._placeholder = placeholder
        obj._epoch_type = epoch_type
        return obj
    def __array_finalize__(self, obj):
        if obj is not None:
            self._placeholder = getattr(obj, '_placeholder', False)
            self._epoch_type = getattr(obj, '_epoch_type', False)
            self._num = getattr(obj, '_num', None)
            self._variable = getattr(obj, '_variable', None)
        self.selection = _selection()
        self._fill()
    def __deepcopy__(self, memo):
        # Ensure that actual data is copied over with the record, not
        # just a placeholder.
        self._fill()
        dup = record(numpy.ndarray.__deepcopy__(self, memo))
        dup._variable = None
        dup._num = None
        dup._placeholder = None
        dup._epoch_type = self._epoch_type
        return dup
    def __setitem__(self, key, value):
        if isinstance(value, list) and len(value) == 1:
            self[key] = value[0]
        elif isinstance(key, tuple) and len(key) > 0:
            if len(key) > 1:
                self[key[0]][key[1:]] = value
            else:
                self[key[0]] = value
        else:
            if self._epoch_type:
                value = typing.epoch(value)
            numpy.ndarray.__setitem__(self, key, value)
    def __getitem__(self, key):
        if isinstance(key, tuple) and len(key) > 0:
            if len(key) > 1:
                return self[key[0]][key[1:]]
            else:
                return self[key[0]]
        else:
            return numpy.ndarray.__getitem__(self, key)

    # Selection closure
    # Record selection implies variable selection and archive selection.
    # Archive selection will take the process-wide CDF selection lock,
    # for thread safety.
    class selection(_selection):
        _record = None
        def __init__(self, *args, **kwargs):
            super(selection, self).__init__(*args, **kwargs)
            self._entered = []
        def __enter__(self, *args, **kwargs):
            super(selection, self).__enter__(*args, **kwargs)
            sel = self._record._variable.selection()
            sel.__enter__()
            self._entered.append(sel)
            internal.CDFlib(
                internal.SELECT_,
                    self._record._variable._tokens['RECORD'],
                        self._record._num + 1)

    def __repr__(self):
        if (self.shape is ()):
            return repr(self[()])
        else:
            return repr(self.tolist())
    # TODO There are many more methods like this.  It would be nice to
    # figure out how to coherently and easily override them all.
#    def _override(fn):
#        def placeholder(obj, *args, **kwargs):
#            if (obj.shape is ()):
#                return obj[()].__dict__[fn](*args, **kwargs)
#            else:
#                return TypeError
#        return placeholder
    def __cmp__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__cmp__(*args, **kwargs)
        else:
            return ValueError
    def __add__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__add__(*args, **kwargs)
        else:
            return ValueError
    def __sub__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__sub__(*args, **kwargs)
        else:
            return ValueError
    def __mul__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__mul__(*args, **kwargs)
        else:
            return ValueError
    def __div__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__div__(*args, **kwargs)
        else:
            return ValueError
    def __radd__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__radd__(*args, **kwargs)
        else:
            return ValueError
    def __rsub__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__rsub__(*args, **kwargs)
        else:
            return ValueError
    def __rmul__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__rmul__(*args, **kwargs)
        else:
            return ValueError
    def __rdiv__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__rdiv__(*args, **kwargs)
        else:
            return ValueError
    def __iadd__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__iadd__(*args, **kwargs)
        else:
            return ValueError
    def __isub__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__isub__(*args, **kwargs)
        else:
            return ValueError
    def __imul__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__imul__(*args, **kwargs)
        else:
            return ValueError
    def __idiv__(self, *args, **kwargs):
        if (self.shape is ()):
            return self[()].__idiv__(*args, **kwargs)
        else:
            return ValueError

    # Internal methods
    def associateVariable(self, variable = None, num = None):
        self._variable = variable
        self._epoch_type = variable._epoch_type
        self._num = num
        self._fill()
    def _dehyper(self, shape, hyper):
        if len(shape) == len(self.shape):
            self[tuple(shape)] = hyper
        else:
            for i in xrange(0, len(hyper)):
                self._dehyper(shape + [i], hyper[i])
    def _hyper(self, shape, index):
        if len(shape) > 0:
            ret = []
            for i in xrange(0, shape[0]):
                ret.append(self._hyper(shape[1:], index + [i]))
            return ret
        else:
            ret = self[tuple(index)]
            if self._epoch_type:
                ret = ret.to_float64()
            return ret
    def _fill(self):
        if self._placeholder and self._variable is not None:
            with self.selection:
                self._epoch_type = (internal.CDFlib(internal.GET_, self._variable._tokens['DATATYPE'])[0] == internal.CDF_EPOCH)
                internal.CDFlib(
                    internal.SELECT_,
                        self._variable._tokens['RECORD'],
                            self._num,
                        self._variable._tokens['RECCOUNT'],
                            1,
                        self._variable._tokens['RECINTERVAL'],
                            1)
                if self._variable._dimSizes is not None:
                    internal.CDFlib(
                        internal.SELECT_,
                            self._variable._tokens['DIMCOUNTS'],
                                self._variable._dimSizes,
                            self._variable._tokens['DIMINTERVALS'],
                                [1 for dim in self._variable._dimSizes],
                            self._variable._tokens['INDEX'],
                                [0 for dim in self._variable._dimSizes])
                (hyper, ) = internal.CDFlib(
                    internal.GET_,
                        self._variable._tokens['HYPER'])
                self._dehyper([], hyper)
                self._placeholder = False
    def _write(self):
        # The variable must have selected itself before calling us.
        with self.selection:
            internal.CDFlib(
                internal.SELECT_,
                    self._variable._tokens['RECORD'],
                        self._num,
                    self._variable._tokens['RECCOUNT'],
                        1,
                    self._variable._tokens['RECINTERVAL'],
                        1)
            if self._variable._dimSizes is not None:
                internal.CDFlib(
                    internal.SELECT_,
                        self._variable._tokens['DIMCOUNTS'],
                            self._variable._dimSizes,
                        self._variable._tokens['DIMINTERVALS'],
                            [1 for dim in self._variable._dimSizes],
                        self._variable._tokens['INDEX'],
                            [0 for dim in self._variable._dimSizes])
            hyper = self._hyper(list(self.shape), [])
            if not isinstance(hyper, list):
                hyper = [hyper]
            internal.CDFlib(
                internal.PUT_,
                    self._variable._tokens['HYPER'],
                        hyper)
    def indices(self):
        # This is a generator function which will iterate over all
        # indices of this variable.  It is suitable for calls to
        # functions which handle one scalar value at a time.
        for (index, value) in numpy.ndenumerate(self):
            if index is ():
                yield None
            else:
                yield index

class variable(list):
    def __init__(self, value = None, archive = None, num = None):
        # Info about this variable
        self._numElementsPerRecord = None
        self._recVariance = None
        self._dimSizes = None
        self._dimVariances = None
        self._num = None
        self._dtype = None
        self._epoch_type = False
        # References
        self._archive = None
        self.attributes = attribute.variableTable(self)
        # Base class constructor
        list.__init__(self)
        # Fill in values from arguments
        if archive is not None:
            self.associateArchive(archive)
            if num is not None:
                self._num = num
                self._meta()
                self._fill()
        if value is not None:
            if isinstance(value, list):
                self.extend(value)
            else:
                self.extend([value])

    # Selection function
    def select(self):
        if self._archive is not None:
            archive = self._archive()
            if archive is not None:
                internal.CDFlib(
                    internal.SELECT_,
                        self._tokens['SELECT_VARIABLE'],
                            self._num)
                return True
        return False
    # Selection closure
    def selection(self):
        class variable_selection(selection):
            def __init__(self, variable):
                self._variable = variable
            def __enter__(self):
                if self._variable is not None:
                    return self._variable.select()
        return variable_selection(self)

    # List method overrides
    def __setitem__(self, key, value):
        list.__setitem__(self, key, self._coerce(value))
        self[key].associateVariable(variable = self, num = key)
    def __setslice__(self, start, end, value):
        raise NotImplemented
    def append(self, item):
        list.append(self, self._coerce(item))
        i = len(self) - 1
        self[i].associateVariable(variable = self, num = i)
    def extend(self, iterable):
        start = len(self)
        list.extend(self, [self._coerce(value) for value in iterable])
        for i in xrange(start, len(self)):
            self[i].associateVariable(variable = self, num = i)
    def insert(self, index, object):
        raise NotImplemented

    # Internal methods
    def _coerce(self, value):
        ret = None
        if isinstance(value, record):
            # Copy, on the theory that this record presently belongs to
            # somebody else.
            ret = copy.deepcopy(value)
        else:
            # Coerce, since I need a record.
            ret = record(input_array = value)
        self._type(ret)
        self._dims(ret)
        return ret
    def _type(self, value):
        if self._dtype is None:
            # Accept the type of this record without question.
            self._dtype = value.dtype
            self._epoch_type = value._epoch_type
        else:
            try:
                # Between the type of the record and the type of the variable,
                # accept the more inclusive type.
                self._dtype = typing.joinNumpyType(value.dtype, self._dtype)
            except:
                # If something went wrong (i.e. it was a string), use the
                # old method
                self._dtype = value.dtype
                self._epoch_type = value._epoch_type
        self._recVariance = internal.VARY
        if self._dtype.type == numpy.string_:
            self._numElementsPerRecord = self._dtype.itemsize
        else:
            self._numElementsPerRecord = 1

    def _dims(self, value):
        self._dimSizes = list(value.shape)
        self._dimVariances = [internal.VARY] * len(self._dimSizes)
    def _meta(self):
        # Fetch interesting information about this variable, but not the
        # data itself.
        if self._archive is not None:
            archive = self._archive()
            if archive is not None:
                internal.CDFlib(
                    internal.SELECT_,
                        self._tokens['SELECT_VARIABLE'],
                            self._num)
                (type, elements, records, recordsVary,
                    dimensionsVary) = internal.CDFlib(
                        internal.GET_,
                            self._tokens['DATATYPE'],
                            self._tokens['NUMELEMS'],
                            self._tokens['MAXREC'],
                            self._tokens['RECVARY'],
                            self._tokens['DIMVARYS'])
                self._dtype = numpy.dtype(typing._typeConversions[type])
                self._epoch_type = (type == internal.CDF_EPOCH)
                self._numElementsPerRecord = elements
                # The default numpy type obtained by simply informing it that
                # we are dealing with strings results in truncating all read
                # strings to 1 character width.
                if self._dtype.type == numpy.string_:
                    self._dtype = numpy.dtype(\
                        '|S' + str(self._numElementsPerRecord))
                self._numRecords = records + 1
                self._recVariance = recordsVary
                self._dimVariances = dimensionsVary

    def _fill(self):
        # Fetch all of the data records for this variable.
        if self._archive is not None:
            internal.CDFlib(
                internal.SELECT_,
                    self._tokens['SELECT_VARIABLE'],
                        self._num)
            if self._numRecords is not None:
                for i in xrange(0, self._numRecords):
                    rec = record(variable = self, num = i)
                    self.append(rec)
            self.attributes.read()
    def _write(self, name):
        # Implemented by subclasses.
        raise NotImplemented
    # Pythonic methods
    def __copy__(self):
        pass
    def __deepcopy__(self, memo):
        # variables are subclassed from dictionaries, and are therefore
        # unhashable.
#        if not self in memo:
            # For some reason, if I do not reference myself, I end up
            # being set to None which makes the copying operation
            # worthless.  Probably it has to do with reference counts
            # while copying.
            hack = repr(self)
            dup = type(self)()
            dup._numElementsPerRecord = self._numElementsPerRecord
            dup._recVariance = self._recVariance
            dup._dimSizes = self._dimSizes
            dup._dimVariances = self._dimVariances
            dup._dtype = self._dtype
            dup._epoch_type = self._epoch_type
            for record in self:
                dup.append(copy.deepcopy(record, memo))
#            memo[self] = dup
#        return memo[self]
            return dup
    # Archive-facing methods
    def associateArchive(self, archive):
        if archive is not None:
            self._archive = weakref.ref(archive)
            self.attributes.notifyAssociation()
#            for record in self:
#                record.associateVariable(self)
        else:
            self.disassociateArchive()
    def disassociateArchive(self):
        self.attributes.notifyDisassociation()
        self._archive = None
    # User-facing methods
    def set(self, value):
        # See what we can do with the value given.
        # We normally cache things as NumPy arrays.
        if isinstance(value, numpy.ndarray):
            return self._set(value)
        else:
            coerce = None
            try:
                coerce = numpy.array(value)
            except TypeError:
                coerce = None
                raise TypeError('Unable to coerce value to array.')
                self._type()
                return False
            else:
                return self._set(coerce)

# The rVariable is not the preferred variable.  rVariables suffer from
# copious restrictions, most notably the requirement for all rVariables
# in a CDF to have the same dimensionality and dimensions.
class rVariable(variable):
    _tokens = {
        # Tokens princiapally used for selecting context of variable.
        'SELECT_VARIABLE':internal.rVAR_,
        'RECORD':internal.rVARs_RECNUMBER_,
        'INDEX':internal.rVARs_DIMINDICES_,
        # Tokens principally used for getting or putting values of variable.
        'DATA':internal.rVAR_DATA_,
        'DATATYPE':internal.rVAR_DATATYPE_,
        'NUMELEMS':internal.rVAR_NUMELEMS_,
        'MAXREC':internal.rVAR_MAXREC_,
        'RECVARY':internal.rVAR_RECVARY_,
        'DIMVARYS':internal.rVAR_DIMVARYS_,
        'RECCOUNT':internal.rVARs_RECCOUNT_,
        'RECINTERVAL':internal.rVARs_RECINTERVAL_,
        'DIMCOUNTS':internal.rVARs_DIMCOUNTS_,
        'DIMINTERVALS':internal.rVARs_DIMINTERVALS_,
        # Tokens princiapally used for selecting context of attribute.
        'NAME':internal.ATTR_NAME_,
        'SELECT_ATTR':internal.ATTR_,
        'SELECT_ENTRY':internal.rENTRY_,
        'ENTRY_EXISTS':internal.rENTRY_EXISTENCE_,
        # Tokens principally used for getting or putting values of attribute.
        'GET_ENTRY':internal.rENTRY_DATA_,
        'GET_ENTRY_DATATYPE':internal.rENTRY_DATATYPE_,
        'GET_ENTRY_NUMELEMS':internal.rENTRY_NUMELEMS_,
        'GET_ATTR_NUMENTRIES':internal.ATTR_NUMrENTRIES_,
        'HYPER':internal.rVAR_HYPERDATA_,
    }
    def __init__(self, value = None, archive = None, num = None):
        # Flags
        # Data
        # Properties
        # Call base class initialization
        variable.__init__(self, value = value, archive = archive, num = num)

    # Internal methods
    def _meta(self):
        variable._meta(self)
        if self._archive is not None:
            archive = self._archive()
            if archive is not None:
                self._dimSizes = archive._dimSizes
    def _write(self, name):
        # The archive should have selected itself before calling us.
        # The archive passes us a name since we don't store that
        # information ourselves.
        self._type(self[0])
        self._dims(self[0])
        type = typing._typeConversions[self._dtype.type]
        if self._epoch_type:
            type = internal.CDF_EPOCH
        (num, ) = internal.CDFlib(
          internal.CREATE_,
            self._tokens['SELECT_VARIABLE'],
              name, type,
              self._numElementsPerRecord,
              self._recVariance, self._dimVariances)
        self._num = num
        # Having created the variable, insert the data.
        internal.CDFlib(
            internal.SELECT_,
                self._tokens['SELECT_VARIABLE'],
                    self._num)
        internal.CDFlib(
            internal.PUT_,
                internal.rVAR_ALLOCATERECS_,
                    len(self))
        for record in self:
            record._write()

# The zVariable is the preferred variable.
class zVariable(variable):
    _tokens = {
        # Tokens princiapally used for selecting context of variable.
        'SELECT_VARIABLE':internal.zVAR_,
        'RECORD':internal.zVARs_RECNUMBER_,
        'INDEX':internal.zVAR_DIMINDICES_,
        # Tokens principally used for getting or putting values of variable.
        'DATA':internal.zVAR_DATA_,
        'DATATYPE':internal.zVAR_DATATYPE_,
        'NUMELEMS':internal.zVAR_NUMELEMS_,
        'MAXREC':internal.zVAR_MAXREC_,
        'RECVARY':internal.zVAR_RECVARY_,
        'DIMVARYS':internal.zVAR_DIMVARYS_,
        'RECCOUNT':internal.zVAR_RECCOUNT_,
        'RECINTERVAL':internal.zVAR_RECINTERVAL_,
        'DIMCOUNTS':internal.zVAR_DIMCOUNTS_,
        'DIMINTERVALS':internal.zVAR_DIMINTERVALS_,
        # Tokens princiapally used for selecting context of attribute.
        'NAME':internal.ATTR_NAME_,
        'SELECT_ATTR':internal.ATTR_,
        'SELECT_ENTRY':internal.zENTRY_,
        'ENTRY_EXISTS':internal.zENTRY_EXISTENCE_,
        # Tokens principally used for getting or putting values of attribute.
        'GET_ENTRY':internal.zENTRY_DATA_,
        'GET_ENTRY_DATATYPE':internal.zENTRY_DATATYPE_,
        'GET_ENTRY_NUMELEMS':internal.zENTRY_NUMELEMS_,
        'GET_ATTR_NUMENTRIES':internal.ATTR_NUMzENTRIES_,
        'HYPER':internal.zVAR_HYPERDATA_,
    }

    # Internal methods
    def _meta(self):
        variable._meta(self)
        (dimSizes, ) = internal.CDFlib(
            internal.GET_,
                internal.zVAR_DIMSIZES_)
        self._dimSizes = dimSizes
    def _write(self, name):
        # The archive should have selected itself before calling us.
        # The archive passes us a name since we don't store that
        # information ourselves.
        if self._dtype.type == numpy.string_:
            # We appear to be composed of strings.  Since the internal
            # library used "numElementsPerRecord" as a string length
            # helper, and since all strings are supposed to be the
            # same length, we should find the longest string length
            # of any of our records and give that as the helper value.
            strlen = self._numElementsPerRecord
            for record in self:
                strlen = max(strlen, record.dtype.itemsize)
            self._numElementsPerRecord = strlen
        dimSizes = self._dimSizes
        if dimSizes is None:
            dimSizes = []
        nDims = len(dimSizes)
        if dimSizes is []:
            dimSizes = [0]
        type = typing._typeConversions[self._dtype.type]
        if self._epoch_type:
            type = internal.CDF_EPOCH
        (num, ) = internal.CDFlib(
          internal.CREATE_,
            self._tokens['SELECT_VARIABLE'],
              name, type,
              self._numElementsPerRecord,
              nDims, dimSizes,
              self._recVariance, self._dimVariances)
        self._num = num
        # Having created the variable, insert the data.
        internal.CDFlib(
            internal.SELECT_,
                self._tokens['SELECT_VARIABLE'],
                    self._num)
        internal.CDFlib(
            internal.PUT_,
                internal.zVAR_ALLOCATERECS_,
                    len(self))
        for record in self:
            record._write()

class archive(dict):
    # Lifecycle methods
    def __init__(self, name = None):
        self.selection._archive = self
        # Call base class initialization
        dict.__init__(self)
        # Declare and null the variables we need.
        self._dimSizes = None
        self._reservation = None
        self._encoding = None
        self._majority = None
        self._format = None

        self._rVariableDeletionNumbers = []
        self._zVariableDeletionNumbers = []
        self._variableInsertions = {}

        # Set up the selection closure, and (only for archives) the
        # creation closure.
#        self.selection = selection()
#        self.creation = selection()

        self.attributes = attribute.archiveTable(self)

        # Initialize the variables in different way depending on
        # whether the file already exists on disk or if we are
        # creating it fresh.
        self._filenames = []
        if name is not None:
            self._open(name)

    # Selection closure
    class selection:
        _archive = None
        def __init__(self, filename = None, create = True):
            self._filename = filename
            if self._filename is None:
#                try:
                    self._filename = self._archive._filenames[-1]
#                except:
#                    pass
            self._id = None
            self._create = create
        def __enter__(self):
            # Stage one: access or create file on disk.
            id = None
            if self._create:
                try:
                    # Default options
                    self._archive._encoding = internal.NETWORK_ENCODING
                    self._archive._majority = internal.ROW_MAJOR
                    self._archive._format = internal.SINGLE_FILE
                    dimSizes = []
                    if self._archive._dimSizes is not None:
                        dimSizes = self._archive._dimSizes
                    # Implicit selection
                    (id, ) = internal.CDFlib(
                        internal.CREATE_,
                            internal.CDF_,
                                os.path.splitext(self._filename)[0],
                                len(dimSizes),
                                (dimSizes if dimSizes is not [] else [0]))
                    # Set some basic properties of the file
                    internal.CDFlib(
                        internal.PUT_,
                            internal.CDF_FORMAT_, self._archive._format,
                            internal.CDF_MAJORITY_, self._archive._majority,
                            internal.CDF_ENCODING_, self._archive._encoding)
                except internal.error:
                    pass
            if id is None:
                # Open CDF
                try:
                    # Implicit selection
                    (id,) = internal.CDFlib(
                        internal.OPEN_,
                            internal.CDF_,
                                os.path.splitext(self._filename)[0])
                except internal.error:
                    pass
            if id is not None:
                # We got an ID.  Save it for the __exit__ call.  We do not
                # yet do anything terribly interesting with it at that
                # time, but it's important to know that we have something
                # open so we close it.
                internal.CDFlib(
                    internal.SELECT_,
                        internal.CDF_,
                            id)
                self._id = id
                return True
            else:
                return False
        def __exit__(self, type, value, traceback):
            if self._id is not None:
                try:
                    internal.CDFlib(
                        internal.CLOSE_,
                            internal.CDF_)
                except internal.error:
                    pass

    # Dictionary method overrides
    def __delitem__(self, key):
        var = dict.__getitem__(self, key)
        if isinstance(var, rVariable):
            self._rVariableDeletionNumbers.append(var._num)
        elif isinstance(var, zVariable):
            self._zVariableDeletionNumbers.append(var._num)
        # Remove the variables reference to us so that it can not
        # propagate changes.
        var.disassociateArchive()
        # In fact, totally destroy the variable.
        del var
    def __setitem__(self, key, value, fromDisk = False):
        if isinstance(key, str):
            coerce = None
            if not isinstance(value, variable):
                # This is not a variable.  Can we coerce it to
                # be a variable?
                coerce = zVariable(value = value)
            else:
                if not fromDisk:
                    coerce = copy.deepcopy(value)
                else:
                    coerce = value
            if isinstance(coerce, rVariable):
                # Preconditions: if this is an rVariable, it must have
                # the same dimensionality as the other rVariables
                # (allowing for non-varying dimensions).  For instance,
                # if all other rVariables in this CDF archive are
                # 3x3x3 cubes, and you come along with a 4x4x4 cube,
                # we're not going to let you into our club.  If you're
                # a 3x3 square with a non-varying dimension, we'll feel
                # pity for you, but we'll let you in.
                if self._dimSizes is None:
                    self._dimSizes = coerce._dimSizes
                elif self._dimSizes != coerce._dimSizes:
                    # TODO implement the vary vs. non-vary checking logic
                    raise TypeError('All rVariables in an archive ' \
                        + 'must have the same dimensions (new: ' \
                        + str(coerce._dimSizes) + ' vs. old: ' \
                        + str(self._dimSizes) + ').')
                    coerce = None
            if coerce is not None:
                # Write-back: assign the reference which pointed to the
                # old incorrect object to point to the new coerced
                # object.  Hopefully this will help the user avoid the
                # error of changing to old object and thinking their
                # changes will be reflected.
                value = coerce
                if archive.__contains__(self, key):
                    archive.__delitem__(self, key)
                coerce.associateArchive(self)
                dict.__setitem__(self, key, coerce)
                if not fromDisk:
                    self._variableInsertions[key] = coerce
        else:
            raise TypeError('Archive keys must be variable names.')

    # Disk-hitting methods
    # Regarding the archive name and the CDF file extension:
    # The CDF library interface including both the standard interface
    # and the internal interface handle file extensions poorly.  I
    # suspect this is due to the support for multi-file CDFs.  Since
    # I have to pass them a name without the .cdf extension in order
    # for them to behave nicely under many circumstances, but I have
    # to use the .cdf extension in order to meaningfully talk about
    # the archive with anything other than the CDF library, I will
    # store the name used to talk with the API as self._cdfname and
    # the name used on disk as self._filename.
    # Note that this system might break down if you name CDFs
    # something other than .cdf, or if you use multi-file CDFs.
    # I'll attempt to adapt it going forward when problems crop up,
    # but right now this is the best I've got.
    def _open(self, filename):
        if filename in self._filenames:
            self._filenames.remove(filename)
        self._filenames.append(filename)
        # Select the archive so that all the attributes and variables
        # have the prerequisite state required to read in their own
        # information.
        try:
            with self.selection(create = False):
                self.attributes.read()
                self._indexVariables()
                return True
        except internal.error:
            # The archive does not exist.  We will not create it at this time.
            return False
    # Flush to disk
    def save(self, filename = None):
        if filename is None:
            if len(self._filenames) == 0:
                raise ValueError('No filename specified and none can be inferred.')
        else:
            if filename in self._filenames:
                self._filenames.remove(filename)
            self._filenames.append(filename)
        # Implicit creation is okay when we are saving, but definitely
        # not when creating.
#        with self.creation:
        with self.selection():
            self._save()
    def _save(self):
        with self.selection():
            # If there have been things removed from the archive then we
            # need to remove them on disk.  Order matters here, because
            # we must refer to the variables by number, and the act of
            # removing a variable causes the renumbering of all subsequent
            # variables.  To avoid unnecessary pain and suffering, we'll
            # remove variables in reverse number order, highest to lowest.
            self._rVariableDeletionNumbers.sort()
            self._rVariableDeletionNumbers.reverse()
            for num in self._rVariableDeletionNumbers:
                internal.CDFlib(
                    internal.SELECT_,
                        internal.rVAR_, num,
                    internal.DELETE_,
                        internal.rVAR_)
            self._rVariableDeletionList = []
            self._zVariableDeletionNumbers.sort()
            self._zVariableDeletionNumbers.reverse()
            for num in self._zVariableDeletionNumbers:
                internal.CDFlib(
                    internal.SELECT_,
                        internal.zVAR_, num,
                    internal.DELETE_,
                        internal.zVAR_)
            self._zVariableDeletionList = []

            # Now write all the new things added to this archive to disk.
            # Prepare for writing variables by pre-selecting intervals for
            # writing.
            for name in self._variableInsertions.keys():
                var = self._variableInsertions[name]
                var._write(name)
            self._variableInsertions = {}

            self.attributes.write()
            for var in self.keys():
                self[var].attributes.write()

    # Access data
    def _indexVariables(self):
        with self.selection():
            # Set some archive-wide properties
            (encoding, format, majority) = internal.CDFlib(
                internal.GET_,
                    internal.CDF_ENCODING_,
                    internal.CDF_FORMAT_,
                    internal.CDF_MAJORITY_)
            self._encoding = encoding
            self._format = format
            self._majority = majority
            # Set some archive-wide properties relevant to rVariables
            (numDims, dimSizes) = internal.CDFlib(
                internal.GET_,
                    internal.rVARs_NUMDIMS_,
                    internal.rVARs_DIMSIZES_)
            if numDims > 0:
                self._dimSizes = dimSizes
            # List all rVariables.
            (count, ) = internal.CDFlib(
                internal.GET_,
                    internal.CDF_NUMrVARS_)
            for num in xrange(0, count):
                internal.CDFlib(
                    internal.SELECT_,
                        internal.rVAR_, num)
                (name, ) = internal.CDFlib(
                    internal.GET_,
                        internal.rVAR_NAME_)
                self.__setitem__(name,
                    rVariable(archive = self, num = num),
                    fromDisk = True)
            # List all zVariables.
            (count, ) = internal.CDFlib(
                internal.GET_,
                    internal.CDF_NUMzVARS_)
            for num in xrange(0, count):
                internal.CDFlib(
                    internal.SELECT_,
                        internal.zVAR_, num)
                (name, ) = internal.CDFlib(
                    internal.GET_,
                        internal.zVAR_NAME_)
                var = zVariable(archive = self, num = num)
                archive.__setitem__(self, name, var, fromDisk = True)
        return True
    # Internal utilities methods
    def _numFromName(self, name):
        if not self._variables:
            self.list()
        if self._variables:
            var = self._variables[name]
            if var:
                return var._num
        return None

if __name__ == '__main__':
    # Demonstrate the powers of CDF!
    # We'll create two CDF files, foo and bar.  We will fill them with
    # random data points and we will write them to disk.  Then we will
    # swap the files on disk, read them back in, and compare, which
    # ought to convince everybody that the reading and writnig utilities
    # are totally functional and that all the information was successfully
    # encoded on the disk itself, not remembered in memory.
    import os
    import os.path
    os.chdir(os.path.dirname(os.tmpnam()))
    foo = cdf("foo")
    bar = cdf("bar")
    # We have CDF files.  Write stuff to them.
    
    # Close the files which will flush all writes to disk.
    foo.close()
    bar.close()
