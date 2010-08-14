# Stock python modules.
import weakref
import xml.dom.minidom

# cdf extension modules.
from .. import internal
import entry

class attribute:
    def __init__(self, parent = None, num = None):
        self._parent = None
        self._num = None
        self._cache = None
        if parent is not None:
            self.adopt(parent)
            if num is not None:
                self._num = num
                self._meta()
                self._fill()
    # TODO Hash by an actual value.  Name would be a decent place to start
    # in our case, but we're not aware of our name.
    def __hash__(self):
        return 1
    # Archive- or variable-facing methods
    def adopt(self, parent):
        if parent is not None:
            self._parent = weakref.ref(parent)
        else:
            self._parent = None
    def _meta(self):
        pass
    def _fill(self):
        pass
    def _write(self):
        pass
    def to_element(self):
        dom = xml.dom.minidom.Document()
        root = dom.createElement('attribute')
        root.setAttribute('name', 'unknown')
        return root

class gAttribute(attribute, list):
    _tokens = {
        # Tokens princiapally used for selecting context.
        'NAME':internal.ATTR_NAME_,
        'SELECT_ATTR':internal.ATTR_,
        'SELECT_ENTRY':internal.gENTRY_,
        # Tokens principally used for getting or putting values.
        'GET_ENTRY':internal.gENTRY_DATA_,
        'GET_ENTRY_DATATYPE':internal.gENTRY_DATATYPE_,
        'GET_ENTRY_NUMELEMS':internal.gENTRY_NUMELEMS_,
        'GET_ATTR_NUMENTRIES':internal.ATTR_NUMgENTRIES_,
    }
    def __init__(self, value = None, archive = None, num = None):
        list.__init__(self)
        attribute.__init__(self, archive, num)
        if value is not None:
            if instance(value, list):
                self.extend(value)
            else:
                self.append(value)
    def _fill(self):
        (count, ) = internal.CDFlib(
            internal.GET_,
                self._tokens['GET_ATTR_NUMENTRIES'])
        for num in xrange(0, count):
            (value, ) = internal.CDFlib(
                internal.SELECT_,
                    self._tokens['SELECT_ATTR'],
                        self._num,
                    self._tokens['SELECT_ENTRY'],
                        num,
                internal.GET_,
                    self._tokens['GET_ENTRY'])
            self.append(value)

class vAttribute(attribute):
    def __init__(self, value = None, variable = None, num = None):
        self._value = None
        attribute.__init__(self, variable, num)
        if value is not None:
            self._value = value
    def _fill(self):
        # Note that vAttributes need not be assigned for each variable.
        # There is, however, absolutely no way to tell a priori whether
        # a variable has an attribute assigned.  An error from the CDF
        # api while attempting to read the attribute value will be
        # interpreted to mean that no such variable is assigned for this
        # attribute.
        try:
            if self._parent is not None:
                parent = self._parent()
                if parent is not None:
                    internal.CDFlib(
                        internal.SELECT_,
                            parent._tokens['SELECT_ATTR'],
                                self._num,
                            parent._tokens['SELECT_ENTRY'],
                                parent._num)
                    # Attempting to read the datatype itself has
                    # proven to be the most reliable way to trigger
                    # an error.
                    (cdfType, ) = internal.CDFlib(
                        internal.GET_,
                            parent._tokens['GET_ENTRY_DATATYPE'])
                    (value, ) = internal.CDFlib(
                        internal.GET_,
                            parent._tokens['GET_ENTRY'])
                    self._value = value
        except:
            self._value = None
    def __repr__(self):
        return repr(self._value)
    def __coerce__(self, other):
        return type(other)(self._value)

class variableTable(dict):
    def __init__(self, variable):
        self._variable = variable
        self._invalid = {}
        dict.__init__(self)
    def __del__(self):
        self.notifyDisassociation()
        dict.__del__(self)
    def __setitem__(self, key, value):
        if key in self:
            dict.__setitem__(self, key, value)
        else:
            archive = self._variable._archive()
            if archive is not None:
                if archive.attributes._reserve(key, self):
                    dict.__setitem__(self, key, value)
                else:
                    self._invalid[key] = value
            else:
                dict.__setitem__(self, key, value)
    def __delitem__(self, key):
        if key in self:
            dict.__delitem__(self, key)
            archive = self._variable._archive()
            if archive:
                archive.attributes._relinquish(key, self)
        else:
            del self._invalid[key]
    def _invalidate(self, key):
        self._invalid[key] = self[key]
        del self[key]
    def _available(self, key):
        if key in self._invalid:
            self[key] = self._invalid[key]
            del self._invalid[key]
    def notifyDisassociation(self):
        archive = self._variable._archive()
        if archive:
            for key in self.keys():
                archive.attributes._relinquish(key, self)
            for key in self._invalid.keys():
                archive.attributes._relinquish(key, self)
    def notifyAssociation(self):
        archive = self._variable._archive()
        if archive:
            migrate = {}
            for key in self._invalid.keys():
                if archive.attributes._reserve(key, self):
                    migrate[key] = self._invalid[key]
                    del self._invalid[key]
            for key in self.keys():
                if not archive.attributes._reserve(key, self):
                    self._invalid[key] = self[key]
                    del self[key]
            self.update(migrate)
    def read(self):
        archive = self._variable._archive()
        if archive is not None:
            for name in archive.attributes._keys():
                self[name] = vAttribute(
                  variable = self._variable,
                  num = archive.attributes._number(name))
    def write(self):
        archive = self._variable._archive()
        if archive is not None:
            for key in self.keys():
                num = archive.attributes._number(key)
                if num is not None:
                    value = entry.entry(self[key])
                    internal.CDFlib(
                        internal.SELECT_,
                            internal.ATTR_,
                                num,
                            self._variable._tokens['SELECT_ENTRY'],
                                self._variable._num,
                        internal.PUT_,
                            self._variable._tokens['GET_ENTRY'],
                                value._cdfType,
                                value._numElements,
                                self[key])

class archiveTable(dict):
    def __init__(self, archive):
        # Reference to the archive we are maintaining variables for.
        # Note that we use weak references to prevent reference cycles
        # so that automatic destruction will work.
        self._archive = weakref.ref(archive)

        # Track which keys have numbers assigned already.
        self._globalScopeNamesToNumbers = {}
        self._variableScopeNamesToNumbers = {}

        # Track which variable tables are currently tracking variable-scope
        # attributes of given keys.
        self._variableScopeUsers = {}
        # Track which variable tables would like to be tracking variable-
        # scope attributes for given keys, but are prevented from doing so
        # by global-scope attributes of the same key.
        self._variableScopeBlockers = {}

        

        # Track which attribute numbers are slated for deletion.  We do not
        # bother keeping tabs on any other information about the attribute
        # because the number is all we need to blow it away.
        self._deletionNumbers = []
        # Track which global attribute keys are slated for creation.  We use
        # the name because a number has not yet been assigned.
        self._creationKeys = []
        self._variableKeys = set()

    def __setitem__(self, key, value, fromDisk = False):
        if key in self._variableScopeUsers:
            for user in self._variableScopeUsers[key]:
                user._invalidate(key)
            del self._variableScopeUsers[key]
        if not fromDisk:
            # This logic implements an update to a key value by removing the
            # key from disk entirely and then writing out all values.
            if key in self and key in self._globalScopeNamesToNumbers:
                self._deletionNumbers.add(self._globalScopeNamesToNumbers[key])
                self._globalScopeNamesToNumbers.remove(key)
            self._creationKeys.append(key)
        dict.__setitem__(self, key, value)
    def __delitem__(self, key):
        if key in self._creationKeys:
            self._creationKeys.remove(key)
        if key in self._globalScopeNamesToNumbers:
            self._deletionNumbers.add(self._globalScopeNamesToNumbers[key])
            self._globalScopeNamesToNumbers.remove(key)
        dict.__delitem__(self, key)
        if key in self._variableScopeBlockers:
            for user in self._variableScopeBlockers[key]:
                user._available(key)
            self._variableScopeBlockers.remove(key)
    def _keys(self):
        return self._variableScopeNamesToNumbers.keys()
    def _number(self, key):
        if key in self._globalScopeNamesToNumbers:
            return None
        elif key not in self._variableScopeNamesToNumbers:
            (num, ) = internal.CDFlib(
                internal.CREATE_,
                    internal.ATTR_,
                        key,
                        internal.VARIABLE_SCOPE)
            self._variableScopeNamesToNumbers[key] = num
        return self._variableScopeNamesToNumbers[key]
    def _reserve(self, key, user):
        if key in self:
            self._variableScopeBlockers.get(key, set()).add(user)
            return False
        else:
            users = self._variableScopeUsers.get(key, [])
            if not user in users:
                users.append(user)
            return True
    def _relinquish(self, key, user):
        try:
            self._variableScopeUsers[key].remove(user)
            if len(self._variableScopeUsers[key]) == 0:
                self._variableScopeUsers.remove(key)
                if key in self._variableScopeNamesToNumbers:
                    self._deletionNumbers.append(
                      self._variableScopeNamesToNumbers[key])
                    self._variableScopeNamesToNumbers.remove(key)
        except:
            # Already removed?
            pass
    def write(self):
        order = self._deletionNumbers[:]
        order.sort()
        order.reverse()
        for num in order:
            internal.CDFlib(
                internal.SELECT_,
                    internal.ATTR_,
                        num,
                internal.DELETE_,
                    internal.ATTR_)
        for key in self._creationKeys:
            value = entry.entry(self[key])
            (num, ) = internal.CDFlib(
                internal.CREATE_,
                    internal.ATTR_,
                        key,
                        internal.GLOBAL_SCOPE)
            internal.CDFlib(
                internal.SELECT_,
                    internal.ATTR_,
                        num,
                    internal.gENTRY_,
                        0,
                internal.PUT_,
                    internal.gENTRY_DATA_,
                        value._cdfType,
                        value._numElements,
                        self[key])
    def read(self):
        (numAttrs, ) = internal.CDFlib(
            internal.GET_,
                internal.CDF_NUMATTRS_)
        for num in xrange(0, numAttrs):
            internal.CDFlib(
                internal.SELECT_,
                    internal.ATTR_,
                        num)
            (name, scope) = internal.CDFlib(
                internal.GET_,
                    internal.ATTR_NAME_,
                    internal.ATTR_SCOPE_)
            if scope == internal.GLOBAL_SCOPE:
                self.__setitem__(\
                    name, gAttribute(archive = self, num = num), True)
            else:
                self._variableScopeNamesToNumbers[name] = num
                self._variableKeys.add(name)
