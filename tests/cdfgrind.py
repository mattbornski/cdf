#!/usr/bin/env python

import cdf
import glob
import os
import subprocess
import sys
import time
import unittest

class MemoryMonitor(object):

    def __init__(self):#, username):
        """Create new MemoryMonitor instance."""
#        self.username = username
        self.pid = os.getpid()

    def usage(self):
        """Return int containing memory used by user's processes."""
        self.process = subprocess.Popen(
#          "ps -u %s -o rss | awk '{sum+=$1} END {print sum}'" % self.username,
          "ps -p %s -o rss | awk '{sum+=$1} END {print sum}'" % self.pid,
          shell=True, stdout=subprocess.PIPE)
        self.stdout_list = self.process.communicate()[0].split('\n')
        return int(self.stdout_list[0])

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

def test(filename):
    results = {}
    monitor = MemoryMonitor()
    baseline = monitor.usage()
    start = time.time()
    # Do something stressful
    c = cdf.archive(filename)
    results['read'] = time.time() - start
    c2 = cdf.archive('tmp')
    for key in c:
        c2[key] = c[key]
        for attribute in c[key].attributes:
            c2[key].attributes[attribute] = c[key].attributes[attribute]
    for attribute in c.attributes:
        c2.attributes[attribute] = c.attributes[attribute]
    c2.save()
    results['write'] = time.time() - start - results['read']
    results['memory'] = monitor.usage() - baseline
    os.remove('tmp.cdf')
    return results

def loop(filename, count=3):
    results = {}
    monitor = MemoryMonitor()
    baseline = monitor.usage()
    for i in xrange(0, count):
        run = test(filename)
        for key in run:
            if key in results:
                results[key].append(run[key])
            else:
                results[key] = [run[key]]
    for key in results:
        print 'Average ' + key + ': \t' + str(sum(results[key]) / len(results[key]))
    print 'Total memory consumption (did the test clean up after itself?'
    print '  ' + str(monitor.usage() - baseline)

if __name__ == '__main__':
    loop(sys.argv[1] if len(sys.argv) > 1 else os.path.splitext(__file__)[0] + '.cdf')
