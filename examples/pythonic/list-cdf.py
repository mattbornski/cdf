#!/usr/bin/env python

import sys
import cdf.pythonic


def list(filename):
    archive = cdf.pythonic.archive(filename)
    print 'Archive "' + filename + '"'
    for attributename in archive.attributes.keys():
        print ' * Attribute "' + attributename + '"'
        print '   = ' + str(archive.attributes[attributename])
    for varname in archive.keys():
        var = archive[varname]
        print ' + Variable "' + varname + '"'
        print '   = ' + str(var)
        for attrname in var.attributes.keys():
            print '    * Attribute "' + attrname + '"'
            print '      = ' + str(var.attributes[attrname])

if __name__ == '__main__':
    for filename in sys.argv[1:]:
        list(filename)
