#!/usr/bin/env python

# cdf extension modules
from .. import internal

class InferenceError(Exception):
    pass

class RedundancyError(Exception):
    pass

attributes = {
  'global':{
    'required':[
      'Project',
      'Source_name',
      'Discipline',
      'Data_type',
      'Descriptor',
      'Data_version',
      'Logical_file_id',
      'PI_name',
      'PI_affiliation',
      'TEXT',
      'Instrument_type',
      'Mission_group',
      'Logical_source',
      'Logical_source_description',
    ],
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
    'required':[
      'CATDESC',
      'DEPEND_0',
      'DEPEND_1',
      'DEPEND_2',
      'DEPEND_3',
      'DISPLAY_TYPE',
      'FIELDNAM',
      'FILLVAL',
      'FORMAT',
      'FORM_PTR',
      'LABLAXIS',
      'LABL_PTR_1',
      'LABL_PTR_2',
      'LABL_PTR_3',
      'UNITS',
      'UNIT_PTR',
      'VALIDMIN',
      'VALIDMAX',
      'VAR_TYPE',
    ],
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

def _locate(apid):
    # Search the system path for a file describing the attributes of
    # the given APID.
    # Return the skeleton structure, if found, or None if not found.
    # TODO actually search path.
    if str(apid) == '243':
        import sys
        sys.path.append('/home/mattborn/rbsp/SOC/modules/pdp/l0-l1/skeletons')
        import apid243
        sys.path.pop()
        return apid243.skeleton
    return None

def autofill(archive):
    pass

def autofill(variable):
    pass
