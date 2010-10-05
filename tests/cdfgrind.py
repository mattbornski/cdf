#!/usr/bin/env python

import cdf
import glob
import os
import sys
import unittest

# If python didn't get compiled with PYDEBUG enabled we can't keep
# track of the reference count.  Returning 1 to avoid zero-devision
# errors.
if not hasattr(sys, 'gettotalrefcount'):
    sys.gettotalrefcount = lambda: 1

def check_test_suite():
    tdir = os.path.join(os.path.dirname(__file__), '..', 'tests')
    if tdir not in sys.path:
        sys.path.insert(0, tdir)
    testfiles = []
    for t in glob.glob(os.path.join(tdir, '*_test.py')):
        testfiles.append(os.path.splitext(os.path.basename(t))[0])
    tests = unittest.TestLoader().loadTestsFromNames(testfiles)
    t = unittest.TextTestRunner(stream=file(os.devnull, 'w'))
    t.run(tests)

def loop(filename, count=10):
    for i in xrange(0, count):
        # Do something stressful

if __name__ == '__main__':
    a = cdf.archive(sys.argv[1])
