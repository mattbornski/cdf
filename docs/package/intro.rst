CDF - an introduction
====================

[CDF]_ is an acronym which refers to both the Common Data Format (as
defined and maintained by NASA GSFC) and this [Python]_ package.

The [CDF]_ package contains three interfaces to deal with [CDF]_ formatted
files.  Two of these are simple mappings from existing interfaces in the
NASA GSFC [CDF]_ libraries with Pythonization (for instance, throwing
exceptions rather than returning error codes.)  Their signatures and usage
are mechanical mappings from their C counterparts.  For more information,
see the sections on:
.. the internal interface
.. the standard interface

The [CDF]_ package contains an additional interface, called the pythonic
interface.  The pythonic interface allows manipulation of the [CDF]_ 
formatted files in a manner more familiar and comfortable to [Python]_
programmers.  Typical usage of the pythonic interface will require no
knowledge of the details of [CDF]_ internal data types or APIs.  For more
information, see the secion on:
.. the pythonic interface
