#!/usr/bin/env python

import csv
import sys
import cdf.pythonic


def copy(filename_from, filename_to):
    file_from = open(filename_from)
    dialect = csv.Sniffer().sniff(file_from.read(1024))
    file_from.seek(0)
    archive_from = csv.DictReader(file_from, dialect = dialect)
    archive_to = cdf.pythonic.archive(filename_to)
    values = {}
    for var in archive_from.fieldnames:
        values[var] = []
    for line in archive_from:
        for var in values.keys():
            values[var].append(float(line[var]))
    for var in values.keys():
        archive_to[var] = values[var]
    file_from.close()

if __name__ == '__main__':
    copy(sys.argv[1], sys.argv[2])
