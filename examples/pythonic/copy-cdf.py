#!/usr/bin/env python

import sys
import cdf.pythonic


def copy(filename_from, filename_to):
    archive_from = cdf.pythonic.archive(filename_from)
    archive_to = cdf.pythonic.archive(filename_to)
    for variablename in archive_from.keys():
        archive_to[variablename] = archive_from[variablename]

if __name__ == '__main__':
    copy(sys.argv[1], sys.argv[2])
