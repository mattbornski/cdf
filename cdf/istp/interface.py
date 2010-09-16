#!/usr/bin/env python

# Stock Python modules.
import os
import os.path
import shutil
import sys
import tempfile

# cdf extension modules.
from .. import interface as cdf
from .. import internal

###
# TODO: The complex interactions of the various strategies are not well
# documented and are in fact quite fragile.  The system evolved as I
# tried to encode all the information about when various ISTP attributes
# are actually required and what they are required to be.  A more studied
# approach to encoding this data might add clarity.
###

# This error indicates that user input is required to fill in some
# missing data.
class InferenceError(Exception):
    pass

# This error indicates that user input is required to resolve
# ambiguous contradictory data.
class RedundancyError(Exception):
    pass

# This error is for internal use, and indicates to the autofilling
# function that there is at this moment insufficient data to
# guess the right value.  There may be sufficient data if we retry
# later.
class _MissingPrerequisite(Exception):
    pass

# This error is for internal use, and indicates to the autofilling
# function that although this var is listed as being potentially
# required, we have determined that it is not.
class _NotRequired(Exception):
    pass

# This error is for internal use, and is not an error at all.  It
# indicates to the autofulling function that this var has been
# inferred correctly and that its requirement is met.
class _InferenceSuccessful(Exception):
    pass


class fillStrategy:
    def __call__(self, archive, attr, var = None):
        return NotImplemented

class userInput(fillStrategy):
    def __call__(self, archive, attr, var = None):
        if var is not None:
            if attr not in archive[var].attributes:
                raise InferenceError
        else:
            if attr not in archive.attributes:
                raise InferenceError

class defaultValue(fillStrategy):
    def __init__(self, value):
        self._value = value
    def __call__(self, archive, attr, var = None):
        if var is not None:
            if attr not in archive[var].attributes:
                archive[var].attributes[attr] = self._value
        else:
            if attr not in archive.attributes:
                archive.attributes[attr] = self._value

class autoIncrement(fillStrategy):
    def __init__(self, value, step = 1):
        self._value = value
        self._step = step
    def __call__(self, archive, attr, var = None):
        if var is not None:
            if attr in archive[var].attributes:
                archive[var].attributes[attr] += self._step
            else:
                archive[var].attributes[attr] = self._value
        else:
            if attr in archive.attributes:
                archive.attributes[attr] += self._step
            else:
                archive.attributes[attr] = self._value

class selectFromList(fillStrategy):
    def __init__(self, choices, default = None):
        self._choices = choices
        self._default = default
    def __call__(self, archive, attr, var = None):
        if var is not None:
            if attr in archive[var].attributes:
                archive[var].attributes[attr] += self._step
            else:
                archive[var].attributes[attr] = self._value
        else:
            if attr in archive.attributes:
                archive.attributes[attr] += self._step
            else:
                archive.attributes[attr] = self._value

class archiveName(fillStrategy):
    def __call__(self, archive, attr, var = None):
        filename = archive._filenames[-1]
        if var is not None:
            if attr not in archive[var].attributes:
                archive[var].attributes[attr] = filename
        else:
            if attr not in archive.attributes:
                archive.attributes[attr] = filename

class varName(fillStrategy):
    def __call__(self, archive, attr, var):
        if attr not in archive[var].attributes:
            archive[var].attributes[attr] = var

class primaryDataOnly(fillStrategy):
    def __call__(self, archive, attr, var):
        if attr not in archive[var].attributes:
            var_type = archive[var].attributes.get('VAR_TYPE', None)
            if var_type == 'data':
                raise InferenceError
            elif var_type is None:
                raise _MissingPrerequisite
            else:
                raise _NotRequired
            

class fillValStrategy(fillStrategy):
    fillvals = {
      internal.CDF_CHAR:    '.',
      internal.CDF_BYTE:    -128,
      internal.CDF_UINT1:    255,
      internal.CDF_UINT2:    65535,
      internal.CDF_UINT4:    4294967295,
      internal.CDF_INT1:    -128,
      internal.CDF_INT2:    -32768,
      internal.CDF_INT4:    -2147483648,
      internal.CDF_REAL4:   -1.0*10**31,
      internal.CDF_REAL8:   -1.0*10**31,
      internal.CDF_EPOCH:   '31-Dec-9999 23:59:59.999',
      internal.CDF_EPOCH16: '31-Dec-9999 23:59:59.999',
    }
    def __call__(self, archive, attr, var):
        if attr not in archive[var].attributes:
            archive[var].attributes[attr] \
              = self.fillvals[archive[var]._cdfType]

class formatStrategy(fillStrategy):
    formats = {
      internal.CDF_CHAR:    '%s',
      internal.CDF_BYTE:    '%c',
      internal.CDF_UINT1:   '%u',
      internal.CDF_UINT2:   '%u',
      internal.CDF_UINT4:   '%lu',
      internal.CDF_INT1:    '%d',
      internal.CDF_INT2:    '%d',
      internal.CDF_INT4:    '%ld',
      internal.CDF_REAL4:   '%f',
      internal.CDF_REAL8:   '%Lf',
      internal.CDF_EPOCH:   '%s',
      internal.CDF_EPOCH16: '%s',
    }
    def __call__(self, archive, attr, var):
        if attr not in archive[var].attributes:
            archive[var].attributes[attr] \
              = self.formats[archive[var]._cdfType]
            raise _InferenceSuccessful

class validminStrategy(fillStrategy):
    fillvals = {
      internal.CDF_CHAR:    '.',
      internal.CDF_BYTE:    -128,
      internal.CDF_UINT1:    255,
      internal.CDF_UINT2:    65535,
      internal.CDF_UINT4:    4294967295,
      internal.CDF_INT1:    -128,
      internal.CDF_INT2:    -32768,
      internal.CDF_INT4:    -2147483648,
      internal.CDF_REAL4:   -1.0*10**31,
      internal.CDF_REAL8:   -1.0*10**31,
      internal.CDF_EPOCH:   '31-Dec-9999 23:59:59.999',
      internal.CDF_EPOCH16: '31-Dec-9999 23:59:59.999',
    }
    def __call__(self, archive, attr, var):
        if attr not in archive[var].attributes:
            archive[var].attributes[attr] \
              = self.fillvals[archive[var]._cdfType]
            raise _InferenceSuccessful

class validmaxStrategy(fillStrategy):
    fillvals = {
      internal.CDF_CHAR:    '.',
      internal.CDF_BYTE:    -128,
      internal.CDF_UINT1:    255,
      internal.CDF_UINT2:    65535,
      internal.CDF_UINT4:    4294967295,
      internal.CDF_INT1:    -128,
      internal.CDF_INT2:    -32768,
      internal.CDF_INT4:    -2147483648,
      internal.CDF_REAL4:   -1.0*10**31,
      internal.CDF_REAL8:   -1.0*10**31,
      internal.CDF_EPOCH:   '31-Dec-9999 23:59:59.999',
      internal.CDF_EPOCH16: '31-Dec-9999 23:59:59.999',
    }
    def __call__(self, archive, attr, var):
        if attr not in archive[var].attributes:
            archive[var].attributes[attr] \
              = self.fillvals[archive[var]._cdfType]
            raise _InferenceSuccessful

class varTypeStrategy(fillStrategy):
    pass

class notRequired(fillStrategy):
    def __call__(self, *args, **kwargs):
        raise _NotRequired

class required(fillStrategy):
    def __init__(self, attr = None):
        self._attr = attr
    def __call__(self, archive, attr, var = None):
        if self._attr is not None:
            attr = self._attr
        if var is not None:
            if attr not in archive[var].attributes:
                raise _MissingPrerequisite
        else:
            if attr not in archive.attributes:
                raise _MissingPrerequisite
        raise _NotRequired

# The contents of this attr must refer to an existing var
# in the archive.
class refersToVariable(fillStrategy):
    def __init__(self, attr = None):
        self._attr = attr
    def __call__(self, archive, attr, var):
        if self._attr is not None:
            attr = self._attr
        if attr not in archive[var].attributes:
            # The attr does not exist.
            raise InferenceError
        elif archive[var].attributes[attr] not in archive:
            # The attr exists but is not valid.
            raise cdf.CoherenceError
        return True

class timeSeriesStrategy(fillStrategy):
    def __call__(self, archive, attr, var):
        var_type = archive[var].attributes.get('VAR_TYPE', None)
        if var_type is None:
            raise _MissingPrerequisite
        elif var_type == 'ignore_data':
            raise _NotRequired
        else:
            display_type = archive[var].attributes.get('DISPLAY_TYPE', None)
            if display_type == 'time_series':
                refersToVariable()(archive, attr, var)
            else:
                raise _NotRequired

class dimensionStrategy(fillStrategy):
    def __init__(self, dim, strategy):
        self._dim = dim
        self._strategy = strategy
    def __call__(self, archive, attr, var):
        # Determine if this strategy applies.
        if len(archive[var]._dimSizes) == self._dim:
            # Call secondary strategy.
            return self._strategy(archive, attr, var)
        else:
            raise _NotRequired

class one_of(fillStrategy):
    def __init__(self, *args):
        self._strategies = args[:]
    def __call__(self, archive, attr, var):
        # Call strategies one by one until something succeeds.
        # If nothing succeeds, return the least traumatic exception we saw.
        exceptions = []
        for strategy in self._strategies:
            try:
                return strategy(archive, attr, var)
            except _MissingPrerequisite as e:
                # Pretty mild, really.
                exceptions.insert(0, e)
            except InferenceError as e:
                exceptions.append(e)
            except RedundancyError as e:
                exceptions.append(e)
            except cdf.CoherenceError as e:
                exceptions.append(e)
            # Do not trap _NotRequired or _InferenceSucceeded, as
            # these will be used at a higher level and are close
            # enough to success that we need not try any other cases.
        if len(exceptions) > 0:
            raise exceptions[0]
        else:
            raise InferenceError

attributes = {
  'global':{
    'required':{
      'Project':                    userInput(),
      'Source_name':                userInput(),
      'Discipline':                 userInput(),
      'Data_type':                  userInput(),
      'Descriptor':                 userInput(),
      'Data_version':               autoIncrement(1),
      'Logical_file_id':            archiveName(),
      'PI_name':                    userInput(),
      'PI_affiliation':             userInput(),
      'TEXT':                       userInput(),
      'Instrument_type':            userInput(),
      'Mission_group':              userInput(),
      'Logical_source':             userInput(),
      'Logical_source_description': userInput(),
    },
    'recommended':[
      'Acknowledgement',
      'ADID_ref',
      'Generated_by',
      'Generation_date',
      'HTTP_LINK',
      'LINK_TEXT',
      'LINK_TITLE',
      'MODS',
      'Rules_of_use',
      'Time_resolution',
    ],
    'optional':[
      'Parents',
      'Skeleton_version',
      'Software_version',
      'TITLE',
      'Validate',
    ],
  },
  'var':{
    'required':{
      'CATDESC':                    varName(),
      'DEPEND_0':                   timeSeriesStrategy(),
      'DEPEND_1':                   dimensionStrategy(1, refersToVariable()),
      'DEPEND_2':                   dimensionStrategy(2, refersToVariable()),
      'DEPEND_3':                   dimensionStrategy(3, refersToVariable()),
      'DISPLAY_TYPE':               notRequired(),
      'FIELDNAM':                   varName(),
      'FILLVAL':                    fillValStrategy(),
      'FORMAT':                     one_of(
                                      refersToVariable('FORM_PTR'),
                                      formatStrategy()),
      'FORM_PTR':                   one_of(
                                      refersToVariable(),
                                      required('FORMAT')),
      'LABLAXIS':                   varName(),
      'LABL_PTR_1':                 one_of(
                                      required('LABLAXIS'),
                                      refersToVariable()),
      'LABL_PTR_2':                 notRequired(),
      'LABL_PTR_3':                 notRequired(),
      'UNITS':                      defaultValue(' '),
      'UNIT_PTR':                   one_of(
                                      required('UNITS'),
                                      refersToVariable()),
      'VALIDMIN':                   one_of(
                                      primaryDataOnly(),
                                      validminStrategy()),
      'VALIDMAX':                   one_of(
                                      primaryDataOnly(),
                                      validmaxStrategy()),
      'VAR_TYPE':                   varTypeStrategy(),
    },
    'recommended':[
      'SCALETYP',
      'SCAL_PTR',
      'VAR_NOTES',
    ],
    'optional':[
      'AVG_TYPE',
      'DELTA_PLUS_VAR',
      'DELTA_MINUS_VAR',
      'DICT_KEY',
      'MONOTON',
      'SCALEMIN',
      'SCALEMAX',
      'V_PARENT',
      'DERIVN',
      'sig_digits',
      'SI_conv',
    ]
  },
}

def autofill(arc, skt):
    dir = tempfile.mkdtemp()
    sktfile = os.path.join(dir, 'sktfile.py')
    shutil.copy(skt, sktfile)
    sys.path.append(dir)
    import sktfile
    required = attributes['global']['required'].keys()
    retry = []
    while len(required) > 0:
        for attr in required:
            try:
                if attr not in arc.attributes:
                    if attr in sktfile.skeleton[0]:
                        arc.attributes[attr] = sktfile.skeleton[0][attr]
                    else:
                        attributes['global']['required'][attr](arc, attr)
                if attr not in arc.attributes:
                    raise InferenceError
            except InferenceError:
                raise InferenceError('Unable to infer value of '
                  + 'global attr "' + str(attr) + '"')
            except _MissingPrerequisite:
                retry.append(attr)
            except _NotRequired:
                # Good enough.
                pass
            except _InferenceSuccessful:
                # Perfect!
                pass
        if len(required) == len(retry):
            # This pass has resolved nothing, abort.
            raise InferenceError('Unable to infer value of '
              + 'global attr "' + str(retry[0]) + '"')
        required = retry
        retry = []
    for var in arc:
        required = attributes['var']['required'].keys()
        retry = []
        while len(required) > 0:
            for attr in required:
                try:
                    if attr not in arc[var].attributes:
                        if attr in sktfile.skeleton[1][var]:
                            arc[var].attributes[attr] \
                              = sktfile.skeleton[1][var][attr]
                        else:
                            attributes['var']['required'][attr](
                              arc, attr, var)
                    if attr not in arc[var].attributes:
                        raise InferenceError
                except InferenceError:
                    raise InferenceError('Unable to infer value of "'
                      + str(attr) + '" for var "' + str(var) + '"')
                except _MissingPrerequisite:
                    retry.append(attr)
                except _NotRequired:
                    # Good enough.
                    pass
                except _InferenceSuccessful:
                    # Perfect!
                    pass
            if len(required) == len(retry):
                # This pass has resolved nothing, abort.
                raise InferenceError('Unable to infer value of "'
                  + str(attr) + '" for var "' + str(var) + '"')
            required = retry
            retry = []

class archive(cdf.archive):
    def __init__(self, *args, **kwargs):
        if 'skeleton' in kwargs:
            self._skeleton = kwargs['skeleton']
            del kwargs['skeleton']
        cdf.archive.__init__(self, *args, **kwargs)
    def _save(self):
        autofill(self, self._skeleton)
        cdf.archive._save(self)
