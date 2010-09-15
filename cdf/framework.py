#!/usr/bin/env python

class hashablyUniqueObject:
    # Typically you do not hash mutable objects.  This hash value, however,
    # is independent of the state of this object.
    def __hash__(self):
        return id(self)
    # We also override the rich comparison operators to be consistent with
    # the uniqueness required of objects which hash uniquely.
    def __lt__(self, other):
        return NotImplemented
    def __le__(self, other):
        return NotImplemented
    def __eq__(self, other):
        return NotImplemented
    def __ne__(self, other):
        return NotImplemented
    def __gt__(self, other):
        return NotImplemented
    def __ge__(self, other):
        return NotImplemented
    # We also override the basic comparison operator for the same reason.
    def __cmp__(self, other):
        return NotImplemented
    # Done with hashing.

class coerciveObject(object):
    allowedTypes = []
    coercedTypes = []
    def _coerce(self, value):
        for type in allowedTypes:
            if isinstance(value, type):
                return value
        for type in coercedTypes:
            try:
                coerced = type(value)
                if coerced is not None:
                    return coerced
            except:
                pass
        return None

class coerciveDictionary(dict):
    pass

class coerciveList(list):
    pass
