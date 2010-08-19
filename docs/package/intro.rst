CDF - an introduction
=====================

CDF is an acronym which refers to both the Common Data Format (as
defined and maintained by NASA GSFC) and this Python package.

The CDF package contains three interfaces to deal with CDF formatted
files.  Two of these are simple mappings from existing interfaces in the
NASA GSFC CDF libraries with Pythonization (for instance, throwing
exceptions rather than returning error codes.)  Their signatures and usage
are mechanical mappings from their C counterparts.

The CDF package contains an additional interface, called the pythonic
interface.  The pythonic interface allows manipulation of the CDF 
formatted files in a manner more familiar and comfortable to Python
programmers.  Typical usage of the pythonic interface will require no
knowledge of the details of CDF internal data types or APIs.
