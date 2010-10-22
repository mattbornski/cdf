#!/usr/bin/env python

import cdf
import glob
import os
import sys
import time
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

def loop(filename, count=3):
    readtimes = []
    writetimes = []
    for i in xrange(0, count):
        start = time.time()
        # Do something stressful
        c = cdf.archive(filename)
        readtimes.append(time.time() - start)
        print 'Read #' + str(len(readtimes)) + ': ' + str(readtimes[-1])
        c2 = cdf.archive('tmp')
        for key in c:
            c2[key] = c[key]
            for attribute in c[key].attributes:
                c2[key].attributes[attribute] = c[key].attributes[attribute]
        for attribute in c.attributes:
            c2.attributes[attribute] = c.attributes[attribute]
        c2.save()
        writetimes.append(time.time() - start - readtimes[-1])
        print 'Write #' + str(len(writetimes)) + ': ' + str(writetimes[-1])
        os.remove('tmp.cdf')
    print 'Average read: ' + str(sum(readtimes) / len(readtimes))
    print 'Average write: ' + str(sum(writetimes) / len(writetimes))

if __name__ == '__main__':
    loop(sys.argv[1])
