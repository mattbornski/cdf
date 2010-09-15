# Stock python modules
import copy
import os
import time
import weakref

# Third-party modules
import numpy

# cdf extension modules
from . import internal
from . import attribute
from . import typing



# Error when a coherence restriction of the CDF format is violated.
class CoherenceError(Exception):
    pass


# Closure to ensure proper selection of CDF files.
class selection:
    def __init__(self):
        pass
    def __enter__(self):
        pass
    def __exit__(self, type, value, traceback):
        pass

# Primitive to underwrite some common functions of CDF data structures.
class primitive:
    def __init__(self, *args, **kwargs):
        self._parents = []
        if 'parent' in kwargs:
            self._parents.append(kwargs['parent'])
    def select(self):
        pass
    def unselect(self):
        pass
    def from_binary(self, *args, **kwargs):
        pass
    def to_binary(self, *args, **kwargs):
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
        if variable is not None:
            obj = numpy.asarray(numpy.zeros(
                shape = variable._dimSizes,
                dtype = variable._numpyType)).view(cls)
        else:
            coerce = None
            if input_array is not None:
                if not isinstance(input_array, numpy.ndarray):
                    # This is not an n-dimensional array.  Can we coerce it to
                    # be an n-dimensional array?
                    try:
                        placeholder = False
                        coerce = numpy.asarray(input_array).view(cls)
                    except TypeError:
                        coerce = None
                        raise TypeError('Record values must be arrays ' \
                            + 'or coercable into arrays.')
                else:
                    coerce = copy.deepcopy(input_array)
            if isinstance(coerce, numpy.ndarray):
                obj = numpy.asarray(coerce).view(cls)
            else:
                obj = numpy.asarray(numpy.zeros(shape = ())).view(cls)
        obj._num = num
        obj._variable = variable
        obj._placeholder = placeholder
        return obj
    def __array_finalize__(self, obj):
        if obj is not None:
            self._placeholder = getattr(obj, '_placeholder', False)
            self._num = getattr(obj, '_num', None)
            self._variable = getattr(obj, '_variable', None)
        self._fill()
    def __deepcopy__(self, memo):
        # Ensure that actual data is copied over with the record, not
        # just a placeholder.
        self._fill()
        dup = record(numpy.ndarray.__deepcopy__(self, memo))
        dup._variable = None
        dup._num = None
        dup._placeholder = None
        return dup

    # Selection function
    def select(self):
        if self._variable is not None:
            if self._variable.select():
                internal.CDFlib(
                    internal.SELECT_,
                        self._variable._tokens['RECORD'],
                            self._num + 1)
                return True
        return False
    # Selection closure
    class selection:
        def __init__(self, record):
            self._record = record
        def __enter__(self):
            if self._record is not None:
                return self._record.select()
            else:
                return False
        def __exit__(self, type, value, traceback):
            pass

    # Pythonic methods
#    def __str__(self):
#        return repr(self)
#    def __repr__(self):
#        if (self.shape is ()):
#            return repr(numpy.asarray(self)[()])
#        else:
#            return repr(numpy.asarray(self))
    # Internal methods
    def associateVariable(self, variable = None, num = None):
        self._variable = variable
        self._num = num
        self._fill()
    def _fill(self):
        if self._placeholder and self._variable is not None:
            with self.selection(self) as selection:
                if selection:
                    internal.CDFlib(
                        internal.SELECT_,
                            self._variable._tokens['RECORD'],
                                self._num)
                    for (index, value) in numpy.ndenumerate(self):
                        if index is not ():
                            internal.CDFlib(
                                internal.SELECT_,
                                    self._variable._tokens['INDEX'],
                                        index)
                        (value, ) = internal.CDFlib(
                            internal.GET_,
                                self._variable._tokens['DATA'])
                        self[index] = value
                self._placeholder = False
    def _write(self):
        # The variable must have selected itself before calling us.
        internal.CDFlib(
            internal.SELECT_,
                self._variable._tokens['RECORD'],
                    self._num)
        for (index, value) in numpy.ndenumerate(self):
            if index is not ():
                internal.CDFlib(
                    internal.SELECT_,
                        self._variable._tokens['INDEX'],
                            index)
            internal.CDFlib(
                internal.PUT_,
                    self._variable._tokens['DATA'], value)
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
        self._cdfType = None
        self._numpyType = None
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
    def __del__(self):
        del self.attr
        list.__del__(self)
        internal.CDFlib(
            internal.CLOSE_,
                self._tokens['SELECT_VARIABLE'])

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

    # Constructor closure
    def _constructor(self):
        return variable

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
            ret = value
        else:
            try:
                ret = record(input_array = value)
            except:
                return None
        self._type(ret)
        self._dims(ret)
        return ret
    def _type(self, value):
        if self._numpyType is None:
            # Accept the type of this record without question.
            self._numpyType = value.dtype.type
        else:
            try:
                # Between the type of the record and the type of the variable,
                # accept the more inclusive type.
                self._numpyType \
                  = typing.joinNumpyType(value.dtype.type, self._numpyType)
            except:
                # If something went wrong (i.e. it was a string), use the
                # old method
                self._numpyType = value.dtype.type
        self._cdfType = typing._typeConversions[self._numpyType]
        self._numElementsPerRecord = 1
        self._recVariance = internal.VARY
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
                self._cdfType = type
                self._numpyType = typing._typeConversions[type]
                self._numElementsPerRecord = elements
                # The default numpy type obtained by simply informing it that
                # we are dealing with strings results in truncating all read
                # strings to 1 character width.
                if self._numpyType == numpy.string_:
                    self._numpyType = numpy.dtype(\
                        '|S' + str(self._numElementsPerRecord))
                self._numRecords = records + 1
                self._recVariance = recordsVary
                self._dimVariances = dimensionsVary
                self._dimSizes = archive._dimSizes

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
            constructor = self._constructor()
            dup = constructor()
            for record in self:
                dup.append(copy.deepcopy(record, memo))
            dup._numElementsPerRecord = self._numElementsPerRecord
            dup._recVariance = self._recVariance
            dup._dimSizes = self._dimSizes
            dup._dimVariances = self._dimVariances
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
        # Tokens princiapally used for selecting context of attribute.
        'NAME':internal.ATTR_NAME_,
        'SELECT_ATTR':internal.ATTR_,
        'SELECT_ENTRY':internal.rENTRY_,
        # Tokens principally used for getting or putting values of attribute.
        'GET_ENTRY':internal.rENTRY_DATA_,
        'GET_ENTRY_DATATYPE':internal.rENTRY_DATATYPE_,
        'GET_ENTRY_NUMELEMS':internal.rENTRY_NUMELEMS_,
        'GET_ATTR_NUMENTRIES':internal.ATTR_NUMrENTRIES_,
    }
    def __init__(self, value = None, archive = None, num = None):
        # Flags
        # Data
        # Properties
        # Call base class initialization
        variable.__init__(self, value = value, archive = archive, num = num)

    # Constructor closure
    def _constructor(self):
        return rVariable

    # Internal methods
    def _write(self, name):
        # The archive should have selected itself before calling us.
        # The archive passes us a name since we don't store that
        # information ourselves.
        self._type(self[0])
        self._dims(self[0])
        (num, ) = internal.CDFlib(
            internal.CREATE_,
                self._tokens['SELECT_VARIABLE'],
                    name, self._cdfType, self._numElementsPerRecord,
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
        # Tokens princiapally used for selecting context of attribute.
        'NAME':internal.ATTR_NAME_,
        'SELECT_ATTR':internal.ATTR_,
        'SELECT_ENTRY':internal.zENTRY_,
        # Tokens principally used for getting or putting values of attribute.
        'GET_ENTRY':internal.zENTRY_DATA_,
        'GET_ENTRY_DATATYPE':internal.zENTRY_DATATYPE_,
        'GET_ENTRY_NUMELEMS':internal.zENTRY_NUMELEMS_,
        'GET_ATTR_NUMENTRIES':internal.ATTR_NUMzENTRIES_,
    }
    def __init__(self, value = None, archive = None, num = None):
        # Call base class initialization
        variable.__init__(self, value = value, archive = archive, num = num)

    # Constructor closure
    def _constructor(self):
        return zVariable

    # Internal methods
    def _write(self, name):
        # The archive should have selected itself before calling us.
        # The archive passes us a name since we don't store that
        # information ourselves.
        if self._cdfType == internal.CDF_CHAR:
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
        (num, ) = internal.CDFlib(
            internal.CREATE_,
                self._tokens['SELECT_VARIABLE'],
                    name, self._cdfType, self._numElementsPerRecord,
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

        self.attributes = attribute.archiveTable(self)

        # Initialize the variables in different way depending on
        # whether the file already exists on disk or if we are
        # creating it fresh.
        self._filenames = []
        if name is not None:
            self._open(name)

    # Selection closure
    class selection:
        def __init__(self, archive, filename, create = True):
            self._archive = archive
            self._filename = filename
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
#                try:
                    coerce = zVariable(value = value)
#                except internal.error:
#                    coerce = None
#                    raise TypeError('Archive values must be variables ' \
#                        + 'or coercable into variables.')
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
        with self.selection(self, filename, create = False) as selection:
            if selection:
                self.attributes.read()
                self._indexVariables()
                return True
            else:
                return False
    # Flush to disk
    def save(self, filename = None):
        if filename is None:
            if len(self._filenames) > 0:
                filename = self._filenames[-1]
            else:
                raise ValueError('No filename specified and none can be inferred.')
        else:
            if filename in self._filenames:
                self._filenames.remove(filename)
            self._filenames.append(filename)
        with self.selection(self, filename) as selection:
            self._save()
    def _save(self):
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
#    def version(self):
#        with self.selection() as selection:
#            (version, ) = internal.CDFlib(
#                internal.GET_,
#                    internal.CDF_VERSION_)
#            return version
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
