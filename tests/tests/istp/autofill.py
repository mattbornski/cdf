#!/usr/bin/env python

def test():
    import cdf
    import cdf.istp
    import os
    import os.path
    import random

    test = cdf.istp.archive('test', skeleton = os.path.splitext(__file__)[0] + '.skt')
    test['independentData'] = [i for i in xrange(0, 5)]
    test['primaryDependentData'] = \
      [random.uniform(0, 100) for i in xrange(0, len(test['independentData']))]
    test['derivedDependentData'] = \
      [0] + [test['primaryDependentData'][i] - test['primaryDependentData'][i - 1] for i in xrange(1, len(test['primaryDependentData']))]
    test.save()

    readback = cdf.archive('test')
    print readback.attributes
    print readback['independentData'].attributes
    print readback['primaryDependentData'].attributes
    print readback['derivedDependentData'].attributes

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
