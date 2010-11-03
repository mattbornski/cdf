#!/usr/bin/env python

def test():
    import cdf
    import cdf.istp
    import os
    import os.path

    test = cdf.istp.archive('test')

    # Invent some variables
    test['onedim'] = [i for i in xrange(0, 5)]
    test['twodim'] = [[i, i + 1] for i in xrange(10, 15)]
    test['threedim'] = [[[i, 2 * i], [i + 1, 2 * i + 1]] for i in xrange(20, 25)]
    # Set their attributes.  In particular, make them dependent on things.

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
