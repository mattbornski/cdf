#!/usr/bin/env python

def test():
    import cdf
    import os

    test = cdf.archive()
    # Test the ability to set lists of mixed-type attributes on the archive.
    test.attributes['one'] = [1, 'one']
    test.attributes['two'] = [2, 2.0]
    test.attributes['three'] = ['three', 3.0]
    test.attributes['four'] = [4.0, 4]
    test.attributes['five'] = [5.0, 'five']
    test.attributes['six'] = ['six', 6]
    # Test the ability to set lists of dimensioned attributes on the archive.
    test.attributes['seven'] = (7, 7)
    test.attributes['eight'] = (8, 8.0)
    test.attributes['nine'] = (9.0, 9)
    test.attributes['ten'] = [(10, 10), (10.0, 10.0), (10, 10.0), (10.0, 10)]
    test.attributes['eleven'] = ('eleven', 'one teen')
    test.save('test')

    test2 = cdf.archive('test')
    print test2.attributes

    os.remove('test.cdf')

if __name__ == '__main__':
    test()
