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

class InferenceError(Exception):
    pass

class RedundancyError(Exception):
    pass



class fillStrategy:
    def __call__(self, archive, attribute, variable = None):
        raise NotImplemented

class userInput(fillStrategy):
    def __call__(self, archive, attribute, variable = None):
        if variable is not None:
            if attribute not in archive[variable].attributes:
                raise InferenceError
        else:
            if attribute not in archive.attributes:
                raise InferenceError

class defaultValue(fillStrategy):
    def __init__(self, value):
        self._value = value
    def __call__(self, archive, attribute, variable = None):
        if variable is not None:
            if attribute not in archive[variable].attributes:
                archive[variable].attributes[attribute] = value
        else:
            if attribute not in archive.attributes:
                archive.attributes[attribute] = value

class autoIncrement(fillStrategy):
    def __init__(self, value, step = 1):
        self._value = value
        self._step = step
    def __call__(self, archive, attribute, variable = None):
        if variable is not None:
            if attribute in archive[variable].attributes:
                archive[variable].attributes[attribute] += self._step
            else:
                archive[variable].attributes[attribute] = self._value
        else:
            if attribute in archive.attributes:
                archive.attributes[attribute] += self._step
            else:
                archive.attributes[attribute] = self._value

class selectFromList(fillStrategy):
    def __init__(self, choices, default = None):
        self._choices = choices
        self._default = default
    def __call__(self, archive, attribute, variable = None):
        if variable is not None:
            if attribute in archive[variable].attributes:
                archive[variable].attributes[attribute] += self._step
            else:
                archive[variable].attributes[attribute] = self._value
        else:
            if attribute in archive.attributes:
                archive.attributes[attribute] += self._step
            else:
                archive.attributes[attribute] = self._value

class archiveName(fillStrategy):
    def __call__(self, archive, attribute, variable = None):
        filename = archive._filenames[-1]
        if variable is not None:
            if attribute not in archive[variable].attributes:
                archive[variable].attributes[attribute] = filename
        else:
            if attribute not in archive.attributes:
                archive.attributes[attribute] = filename

class primaryDataOnly(fillStrategy):
    pass

class validminStrategy(fillStrategy):
    pass

class validmaxStrategy(fillStrategy):
    pass

class varTypeStrategy(fillStrategy):
    pass

class notRequired(fillStrategy):
    pass

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
  'variable':{
    # TODO some of these attributes are only "required" if you are a
    # particular type of variable, and some of them are redundant and
    # should be disallowed from coexisting.  Figure out a way to encode
    # this.
    # TODO some of these variables values can be inferred reasonably well.
    # Figure out a way to encode this.
    'required':{
      'CATDESC':                    userInput(),
      'DEPEND_0':                   primaryDataOnly(),
      'DEPEND_1':                   notRequired(),
      'DEPEND_2':                   notRequired(),
      'DEPEND_3':                   notRequired(),
      'DISPLAY_TYPE':               notRequired(),
      'FIELDNAM':                   notRequired(),
      'FILLVAL':                    notRequired(),
      'FORMAT':                     notRequired(),
      'FORM_PTR':                   notRequired(),
      'LABLAXIS':                   notRequired(),
      'LABL_PTR_1':                 notRequired(),
      'LABL_PTR_2':                 notRequired(),
      'LABL_PTR_3':                 notRequired(),
      'UNITS':                      notRequired(),
      'UNIT_PTR':                   notRequired(),
      'VALIDMIN':                   validminStrategy(),
      'VALIDMAX':                   validmaxStrategy(),
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

def autofill(arc, skt):
    dir = tempfile.mkdtemp()
    sktfile = os.path.join(dir, 'sktfile.py')
    shutil.copy(skt, sktfile)
    sys.path.append(dir)
    import sktfile
    for attr in attributes['global']['required']:
        try:
            if attr not in arc.attributes:
                if attr in sktfile.skeleton[0]:
                    arc.attributes[attr] = sktfile.skeleton[0][attr]
                else:
                    attributes['global']['required'][attr](arc, attr)
            if attr not in arc.attributes:
                raise InferenceError
        except InferenceError:
            raise InferenceError(attr)
        

class archive(cdf.archive):
    def __init__(self, *args, **kwargs):
        if 'skeleton' in kwargs:
            self._skeleton = kwargs['skeleton']
            del kwargs['skeleton']
        cdf.archive.__init__(self, *args, **kwargs)
    def _save(self):
        autofill(self, self._skeleton)
        cdf.archive._save(self)
