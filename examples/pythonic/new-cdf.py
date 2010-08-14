#!/usr/bin/env python

import sys
import time
import cdf.pythonic


def new(filename):
    now = time.time()
    archive = cdf.pythonic.archive(filename)
    archive.attr['timezone'] = time.tzname[0]
    archive['unixtime'] = [now]
    archive['unixtime'].attr['units'] = 's'
    archive['localtime'] = [time.asctime(time.localtime(now))]
    archive['utctime'] = [time.asctime(time.gmtime(now))]
    del archive

if __name__ == '__main__':
    new(sys.argv[1])
